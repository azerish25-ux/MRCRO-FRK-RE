"""Execute the recovered VAnalog output wrapper in bounded offline emulation.

Fixtures explicitly supply engine parameters, interpolation history and level.
They do not establish complete factory initialization or UI control conversion.
No second synth is launched; live audio observations use the already-open app.
"""
import argparse
import json
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from native_reference import ProcessorReference, PROCESSOR_SHA256


def bits(value):
    return struct.unpack("<I", struct.pack("<f", value))[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("processor", type=Path)
    parser.add_argument("--vanalog-reference", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if output.drive.upper() != "D:":
        parser.error("local reference evidence must remain on D:")
    reference = ProcessorReference(args.processor)
    machine = reference.machine
    wrapper, allocator, temporary, destination = (reference.heap + n * 4096 for n in range(1, 5))
    source = json.loads(args.vanalog_reference.read_text(encoding="utf-8"))
    if source["processor_sha256"] != PROCESSOR_SHA256 or source["frames_per_block"] != 16:
        parser.error("expected original processor's 16-frame VAnalog traces")
    scenarios = []
    for scenario in source["scenarios"]:
        machine.mem_write(wrapper, bytes(512))
        machine.mem_write(wrapper, struct.pack("<Q", 0x180545F18))
        machine.mem_write(wrapper + 0x38, struct.pack("<Q", 0x180540CF0))
        machine.mem_write(allocator, struct.pack("<QQQ", temporary, 128, 128))
        machine.mem_write(temporary, bytes(128))
        reference.call(0x18032FE50, (wrapper + 0x38, allocator))
        initial_previous_bits = bits(-0.25)
        machine.mem_write(wrapper + 0xC8, struct.pack("<I", initial_previous_bits))
        blocks = []
        levels = (0.5, 0.0, 1.0, 0.8124999403953552, -0.25, 0.125, 0.0, 1.0)
        for index, block in enumerate(scenario["blocks"]):
            parameter_bits = block["parameter_bits_note_timbre_morph_harmonics"]
            machine.mem_write(wrapper + 0xD0,
                              struct.pack("<i4If", 2, *parameter_bits, 1.0))
            level_bits = bits(levels[index % len(levels)])
            machine.mem_write(wrapper + 0x28, struct.pack("<I", level_bits))
            machine.mem_write(destination, bytes(128))
            # Calls the original virtual output wrapper, raw engine, interpolation
            # and final level multiplication without replacing any instructions.
            reference.call(0x1802C5380, (wrapper, 0, destination))
            blocks.append({"parameters_bits": parameter_bits, "level_bits": level_bits,
                           "output_bits": list(struct.unpack("<32I", machine.mem_read(destination, 128)))})
        scenarios.append({"name": scenario["name"], "initial_previous_bits": initial_previous_bits,
                          "blocks": blocks})
    result = {"processor_sha256": PROCESSOR_SHA256,
              "wrapper_address": "0x1802c5380", "virtual_render_address": "0x1802c54b0",
              "unicorn_version": reference.unicorn_version,
              "method": "unchanged original wrapper instructions with explicitly initialized fixture fields",
              "scenarios": scenarios,
              "limitations": ["fixture initialization is explicit, not the full factory constructor",
                              "UI/control word conversion, voice/filter/effect routing excluded",
                              "not live audio or proof of complete instrument parity"]}
    with output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, indent=2)
        stream.write("\n")
    print(json.dumps({"scenarios": len(scenarios),
                      "output_samples": sum(len(s["blocks"]) * 32 for s in scenarios)}, indent=2))


if __name__ == "__main__":
    main()
