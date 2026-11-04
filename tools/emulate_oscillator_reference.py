"""Observe bounded numeric blocks from the recovered VAnalog engine in emulation.

This does not open another synth or capture instrument audio. It emulates the
identified processor's Init/Render routines for numerical reconstruction work.
Live audible reference observations use the user's already-open synth.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from scan_pe_strings import pe_layout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("processor", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if output.drive.upper() != "D:":
        parser.error("local numerical evidence must remain on D:")
    data = args.processor.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != "bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74":
        parser.error("processor identity does not match recovered routine addresses")
    from unicorn import Uc, UC_ARCH_X86, UC_MODE_64, __version__
    from unicorn.x86_const import (UC_X86_REG_RSP, UC_X86_REG_RIP, UC_X86_REG_RCX,
                                   UC_X86_REG_RDX, UC_X86_REG_R8, UC_X86_REG_R9)

    machine = Uc(UC_ARCH_X86, UC_MODE_64)
    image_base, sections = pe_layout(data)
    size = (max(s["rva"] + s["size"] for s in sections) + 4095) & ~4095
    machine.mem_map(image_base, size)
    for section in sections:
        machine.mem_write(image_base + section["rva"],
                          data[section["file_offset"]:section["file_offset"] + section["size"]])
    stack, sentinel, heap = 0x200000000, 0x300000000, 0x400000000
    machine.mem_map(stack, 65536)
    machine.mem_map(sentinel, 4096)
    machine.mem_map(heap, 65536)
    engine, allocator, temp, parameters, out, aux = (heap + n * 4096 for n in range(1, 7))
    machine.mem_write(engine, struct.pack("<Q", 0x180540CF0))
    machine.mem_write(allocator, struct.pack("<QQQ", temp, 128, 128))

    def invoke(address, rcx, rdx=0, r8=0, r9=0, stack_arguments=()):
        pointer = stack + 65536 - 256 - 8
        machine.mem_write(pointer, bytes(256))
        machine.mem_write(pointer, struct.pack("<Q", sentinel))
        for index, argument in enumerate(stack_arguments):
            machine.mem_write(pointer + 40 + index * 8, struct.pack("<Q", argument))
        for register, value in ((UC_X86_REG_RSP, pointer), (UC_X86_REG_RCX, rcx),
                                (UC_X86_REG_RDX, rdx), (UC_X86_REG_R8, r8), (UC_X86_REG_R9, r9)):
            machine.reg_write(register, value)
        machine.emu_start(address, sentinel, timeout=2000000, count=1000000)
        if machine.reg_read(UC_X86_REG_RIP) != sentinel:
            raise RuntimeError("routine did not return within its emulation budget")

    invoke(0x18032FE50, engine, allocator)
    machine.mem_write(parameters, struct.pack("<ifffff", 2, 69.0, 0.3, 0.4, 0.5, 1.0))
    blocks = []
    for index in range(4):
        invoke(0x18032FEF0, engine, parameters, out, aux, (32, heap + 7 * 4096))
        out_bytes = bytes(machine.mem_read(out, 128))
        aux_bytes = bytes(machine.mem_read(aux, 128))
        blocks.append({"block_index": index,
                       "out_bits": list(struct.unpack("<32I", out_bytes)),
                       "aux_bits": list(struct.unpack("<32I", aux_bytes))})
    result = {"processor_sha256": digest, "unicorn_version": __version__,
              "method": "bounded emulation of original Init/Render and original tables",
              "init_address": "0x18032fe50", "render_address": "0x18032fef0",
              "parameters": {"trigger": 2, "note": 69.0, "timbre": 0.3,
                             "morph": 0.4, "harmonics": 0.5, "accent": 1.0},
              "frames_per_block": 32, "blocks": blocks,
              "limitations": ["one engine input and four blocks only",
                              "not live sound or a complete instrument render",
                              "wrapper pitch/rate/parameter routing remains unverified"]}
    with output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, indent=2)
        stream.write("\n")
    print(json.dumps({"blocks": len(blocks), "samples": len(blocks) * 32,
                      "first_samples": struct.unpack("<8f", machine.mem_read(out, 32))}, indent=2))


if __name__ == "__main__":
    main()
