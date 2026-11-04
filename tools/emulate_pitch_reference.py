"""Generate pure-helper reference vectors by emulating original x64 instructions.

This is not an audio render or execution of the synth application. The explicit
address is valid only for the processor hash checked below. No Windows DLL is
loaded. Only a bounded, table-based pitch conversion helper is emulated.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from scan_pe_strings import pe_layout


EXPECTED_SHA256 = "bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74"
FUNCTION_ADDRESS = 0x18032E950


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("processor", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if output.drive.upper() != "D:":
        parser.error("local reference evidence must remain on D:")
    data = args.processor.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXPECTED_SHA256:
        parser.error("processor identity does not match the recovered helper address")
    from unicorn import Uc, UC_ARCH_X86, UC_MODE_64, __version__
    from unicorn.x86_const import UC_X86_REG_RSP, UC_X86_REG_RIP, UC_X86_REG_XMM0

    image_base, sections = pe_layout(data)
    image_size = (max(s["rva"] + s["size"] for s in sections) + 4095) & ~4095
    machine = Uc(UC_ARCH_X86, UC_MODE_64)
    machine.mem_map(image_base, image_size)
    for section in sections:
        machine.mem_write(image_base + section["rva"],
                          data[section["file_offset"]:section["file_offset"] + section["size"]])
    stack = 0x200000000
    sentinel = 0x300000000
    machine.mem_map(stack, 65536)
    machine.mem_map(sentinel, 4096)
    values = [-200.0, -119.0, -118.999, -60.0, 0.0, 8.999, 9.0, 12.0, 48.0,
              60.0, 60.001, 60.003, 60.004, 60.5, 69.0, 69.125, 96.0, 127.0,
              135.999, 136.0, 200.0]
    vectors = []
    for value in values:
        input_bits = struct.unpack("<I", struct.pack("<f", value))[0]
        stack_pointer = stack + 65536 - 40
        machine.mem_write(stack_pointer, struct.pack("<Q", sentinel))
        machine.reg_write(UC_X86_REG_RSP, stack_pointer)
        machine.reg_write(UC_X86_REG_XMM0, input_bits)
        machine.emu_start(FUNCTION_ADDRESS, sentinel, timeout=1000000, count=10000)
        if machine.reg_read(UC_X86_REG_RIP) != sentinel:
            raise RuntimeError("helper did not return within its instruction/time budget")
        result_bits = machine.reg_read(UC_X86_REG_XMM0) & 0xFFFFFFFF
        vectors.append({"input_bits": input_bits, "output_bits": result_bits,
                        "input_float": struct.unpack("<f", struct.pack("<I", input_bits))[0],
                        "output_float": struct.unpack("<f", struct.pack("<I", result_bits))[0]})
    result = {"processor_sha256": EXPECTED_SHA256, "function_address": hex(FUNCTION_ADDRESS),
              "method": "Unicorn x86-64 emulation of original helper and file-backed tables",
              "unicorn_version": __version__, "vectors": vectors,
              "limitations": ["selected finite float inputs only", "one pure helper, no instrument audio",
                              "emulation is not an observation of the running Windows process"]}
    with output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, indent=2)
        stream.write("\n")
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
