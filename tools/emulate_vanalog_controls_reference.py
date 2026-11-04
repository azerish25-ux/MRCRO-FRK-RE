"""Observe original VAnalog control-word and trigger-latch state transitions."""
import argparse
import json
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from native_reference import ProcessorReference, PROCESSOR_SHA256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("processor", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if output.drive.upper() != "D:":
        parser.error("local evidence must remain on D:")
    reference = ProcessorReference(args.processor)
    machine = reference.machine
    state = reference.heap + 4096
    coefficient = struct.unpack("<I", machine.mem_read(0x180555EAC, 4))[0]
    initial_parameters = list(struct.unpack("<4I", struct.pack("<4f", 69.0, 0.75, 0.125, 0.25)))
    word_values = (0, 1, 16383, 32766, 65532, 65535, 42, 32768)
    signals = (1, 1, 0, 1, 0, 2, 255, 0)
    scenarios = []
    for mask in range(8):
        machine.mem_write(state, bytes(512))
        initial_latched = (0, 1, 2, 255)[mask % 4]
        machine.mem_write(state + 0x31, bytes([initial_latched]))
        machine.mem_write(state + 0xD0, struct.pack("<i4I", 77, *initial_parameters))
        steps = []
        for index, signal in enumerate(signals):
            words = [word_values[(index + control + mask) % len(word_values)] for control in range(3)]
            dirty = [(1, 2, 255)[(index + control) % 3] if mask & (1 << control) else 0 for control in range(3)]
            machine.mem_write(state + 8, struct.pack("<3H", *words))
            machine.mem_write(state + 0xE, bytes(dirty))
            machine.mem_write(state + 0x30, bytes([signal]))
            reference.call(0x1802C5570, (state,))
            steps.append({"words": words, "dirty": dirty, "signal": signal,
                          "parameters_after": list(struct.unpack("<4I", machine.mem_read(state + 0xD4, 16))),
                          "trigger_after": struct.unpack("<i", machine.mem_read(state + 0xD0, 4))[0],
                          "signal_after": machine.mem_read(state + 0x30, 1)[0],
                          "latched_after": machine.mem_read(state + 0x31, 1)[0],
                          "dirty_after": list(machine.mem_read(state + 0xE, 3)),
                          "words_after": list(struct.unpack("<3H", machine.mem_read(state + 8, 6)))})
        scenarios.append({"name": "dirty_mask_" + str(mask), "initial_latched": initial_latched,
                          "initial_trigger": 77, "initial_parameters": initial_parameters, "steps": steps})
    result = {"processor_sha256": PROCESSOR_SHA256, "routine_address": "0x1802c5570",
              "scale_address": "0x180555eac", "scale_bits": coefficient,
              "method": "bounded emulation of original control update instructions and data",
              "scenarios": scenarios,
              "limitations": ["caller-supplied state and selected word/flag sequences",
                              "does not establish upstream GUI/MIDI scheduler behavior or factory defaults"]}
    with output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, indent=2)
        stream.write("\n")
    print(json.dumps({"scenarios": len(scenarios), "updates": sum(len(s["steps"]) for s in scenarios),
                      "scale_bits": hex(coefficient)}, indent=2))


if __name__ == "__main__":
    main()
