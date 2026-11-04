"""Generate independent 2x interpolation and VAnalog-chain numerical references.

Expected outputs come from the installed processor's instructions in Unicorn,
not from the replacement C++ implementation. This is not a live audio capture.
"""
import argparse
import json
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from native_reference import ProcessorReference, PROCESSOR_SHA256


def float_bits(value):
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
    state, input_pointer, output_pointer = (reference.heap + n * 4096 for n in range(1, 4))
    input_cases = [
        ("silence", -0.0, [[0.0] * 16] * 4),
        ("impulse", 0.0, [[1.0] + [0.0] * 15, [0.0] * 16, [-1.0] + [0.0] * 15, [0.0] * 16]),
        ("ramp", -0.75, [[(index - 8) / 8 for index in range(16)],
                         [(8 - index) / 8 for index in range(16)]] * 2),
        ("alternating", 0.5, [[(-1.0 if index % 2 else 1.0) * level for index in range(16)]
                              for level in (1.0, 0.5, 0.125, 0.0)]),
        ("rounding", struct.unpack("<f", struct.pack("<I", 0x3F000001))[0],
         [[struct.unpack("<f", struct.pack("<I", value))[0] for value in
           (0x3F000001, 0x3F000002, 0xBF000001, 0x3EFFFFFF,
            0x3F7FFFFF, 0xBF7FFFFF, 0x33800000, 0xB3800000,
            0x00000000, 0x80000000, 0x3DCCCCCD, 0xBDCCCCCD,
            0x3F000003, 0x3EFFFFFE, 0xBF000003, 0xBEFFFFFE)]] * 4),
    ]
    scenarios = []

    def observe(name, previous_bits, blocks, pipeline):
        machine.mem_write(state, struct.pack("<I", previous_bits))
        observed = []
        for block in blocks:
            machine.mem_write(input_pointer, struct.pack("<16I", *block["input_bits"]))
            machine.mem_write(output_pointer, bytes(128))
            reference.call(0x1802C9E60, (state, input_pointer, output_pointer))
            observed.append({**block,
                "output_bits": list(struct.unpack("<32I", machine.mem_read(output_pointer, 128))),
                "final_previous_bits": struct.unpack("<I", machine.mem_read(state, 4))[0]})
        scenarios.append({"name": name, "initial_previous_bits": previous_bits,
                          "vanalog_pipeline": pipeline, "blocks": observed})

    for name, previous, blocks in input_cases:
        observe(name, float_bits(previous), [{"input_bits": [float_bits(value) for value in block],
                                              "parameters_bits": [0, 0, 0, 0]}
                                             for block in blocks], False)
    vanalog = json.loads(args.vanalog_reference.read_text(encoding="utf-8"))
    if vanalog["processor_sha256"] != PROCESSOR_SHA256 or vanalog["frames_per_block"] != 16:
        parser.error("VAnalog reference must be the same processor's 16-frame trace")
    for scenario in vanalog["scenarios"]:
        observe("vanalog/" + scenario["name"], float_bits(0.0),
                [{"input_bits": block["out_bits"],
                  "parameters_bits": block["parameter_bits_note_timbre_morph_harmonics"]}
                 for block in scenario["blocks"]], True)
    result = {"processor_sha256": PROCESSOR_SHA256, "routine_address": "0x1802c9e60",
              "unicorn_version": reference.unicorn_version,
              "method": "bounded emulation of original interpolation instructions",
              "scenarios": scenarios,
              "limitations": ["finite selected input sequences and caller-supplied initial state",
                              "pipeline excludes wrapper controls, gain, voices, filters and effects",
                              "not live sound or proof of complete instrument behavior"]}
    with output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, indent=2)
        stream.write("\n")
    print(json.dumps({"scenarios": len(scenarios),
                      "output_samples": sum(len(s["blocks"]) * 32 for s in scenarios),
                      "pipeline_samples": sum(len(s["blocks"]) * 32 for s in scenarios if s["vanalog_pipeline"])}, indent=2))


if __name__ == "__main__":
    main()
