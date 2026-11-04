"""Generate Basic Waves references through original initialization and processing.

Mathematical table data is independently verified original-state input. This is
bounded offline emulation, not a second synth instance or live sound capture.
"""
import argparse
import json
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from native_reference import ProcessorReference, PROCESSOR_SHA256
from basic_waves_reference import populate_verified_tables


def float_bits(value):
    return struct.unpack("<I", struct.pack("<f", value))[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("processor", type=Path)
    parser.add_argument("--table-evidence", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if output.drive.upper() != "D:":
        parser.error("local numerical evidence must remain on D:")
    reference = ProcessorReference(args.processor)
    table_hashes = populate_verified_tables(reference, args.table_evidence)
    machine, registers = reference.machine, reference.registers
    engine, destination = reference.heap + 4096, reference.heap + 8192
    scenarios = []
    settings = [("initial", 440.0, 69.0, 0.125, (0, 0, 0)),
                ("center", 440.0, 69.0, 0.0, (32766, 32766, 32766)),
                ("high", 1760.0, 93.0, 0.25, (65532, 65532, 65532)),
                ("low", 55.0, 33.0, 0.75, (16383, 49149, 32766)),
                ("settled", 440.0, 69.0, 0.75, (65532, 65532, 65532)),
                ("word_overflow", 440.0, 69.0, 0.125, (65535, 65535, 65535)),
                ("phase_wrap", 440.0, 69.0, 1.0, (32766, 0, 0)),
                ("pitch_changes", 110.0, 45.0, 0.25, (32766, 32766, 32766)),
                ("preserved_controls", 440.0, 69.0, 0.125, (32766, 32766, 32766)),
                ("reverse_frequency", -440.0, 69.0, 0.25, (0, 0, 0))]
    def set_pitch(frequency, table_key):
        machine.reg_write(registers.UC_X86_REG_XMM1, float_bits(frequency))
        machine.reg_write(registers.UC_X86_REG_XMM2, float_bits(table_key))
        reference.call(0x1802C6870, (engine,))

    for name, frequency, table_key, phase, words in settings:
        machine.mem_write(engine, bytes(512))
        reference.call(0x1802EA3F0, (engine, 0))
        machine.mem_write(engine, struct.pack("<Q", 0x1805449E0))
        set_pitch(frequency, table_key)
        machine.reg_write(registers.UC_X86_REG_XMM1, float_bits(phase))
        reference.call(0x1802C6850, (engine,))
        warm_controls = name in ("settled", "word_overflow")
        if warm_controls:
            machine.mem_write(engine + 8, struct.pack("<3H", *words))
            machine.mem_write(engine + 0xE, bytes([1, 1, 1]))
            reference.call(0x1802CF8C0, (engine,))
        reference.call(0x1802CF840, (engine,))
        blocks = []
        for index in range(4):
            current_words = words
            dirty = [1, 1, 1]
            changed_pitch = name == "pitch_changes" and index > 0
            next_frequency, next_key = ((220.0, 57.0), (880.0, 81.0), (55.0, 33.0))[index - 1] if changed_pitch else (frequency, table_key)
            if changed_pitch:
                set_pitch(next_frequency, next_key)
            if name == "preserved_controls" and index > 0:
                current_words = (65532, 0, 65535)
                dirty = ([0, 0, 0], [0, 1, 0], [1, 0, 1])[index - 1]
            machine.mem_write(engine + 8, struct.pack("<3H", *current_words))
            machine.mem_write(engine + 0xE, bytes(dirty))
            reference.call(0x1802CF8C0, (engine,))
            reference.call(0x1802CFB30, (engine, destination))
            blocks.append({"words": list(current_words), "dirty": dirty,
                           "set_pitch": changed_pitch, "frequency_bits": float_bits(next_frequency),
                           "table_key_bits": float_bits(next_key),
                           "output_bits": list(struct.unpack("<32I", machine.mem_read(destination, 128)))})
        scenarios.append({"name": name, "frequency_bits": float_bits(frequency),
                          "table_key_bits": float_bits(table_key), "phase_bits": float_bits(phase),
                          "warm_controls": warm_controls, "initial_words": list(words),
                          "blocks": blocks})
    result = {"processor_sha256": PROCESSOR_SHA256, "table_inputs": table_hashes,
              "method": "original base initialization, pitch/phase methods, initialization, control update and raw render",
              "scenarios": scenarios,
              "limitations": ["explicit method order and fixture inputs; full host scheduling not established",
                              "raw output excludes wrapper level, voices, filters, effects and presets",
                              "not live sound or full instrument parity"]}
    with output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, indent=2)
        stream.write("\n")
    nonzero = sum(any(bits & 0x7FFFFFFF for bits in block["output_bits"])
                  for scenario in scenarios for block in scenario["blocks"])
    print(json.dumps({"scenarios": len(scenarios), "samples": len(scenarios) * 4 * 32,
                      "nonzero_blocks": nonzero}, indent=2))
    if nonzero == 0:
        raise RuntimeError("silent fixture set is insufficient for oscillator reconstruction")


if __name__ == "__main__":
    main()
