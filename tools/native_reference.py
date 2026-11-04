"""Bounded offline execution of selected routines in the identified processor.

No Windows DLL is loaded and no synth instance is launched. Callers supply
explicit recovered addresses, memory and arguments for numerical evidence.
"""
import hashlib
from pathlib import Path
import struct

from scan_pe_strings import pe_layout


PROCESSOR_SHA256 = "bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74"


class ProcessorReference:
    def __init__(self, processor):
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_64, __version__
        from unicorn import x86_const

        data = Path(processor).read_bytes()
        if hashlib.sha256(data).hexdigest() != PROCESSOR_SHA256:
            raise ValueError("processor identity does not match recovered routine addresses")
        image_base, sections = pe_layout(data)
        image_size = (max(s["rva"] + s["size"] for s in sections) + 4095) & ~4095
        self.machine = Uc(UC_ARCH_X86, UC_MODE_64)
        self.machine.mem_map(image_base, image_size)
        for section in sections:
            self.machine.mem_write(image_base + section["rva"],
                data[section["file_offset"]:section["file_offset"] + section["size"]])
        self.stack, self.sentinel, self.heap = 0x200000000, 0x300000000, 0x400000000
        self.machine.mem_map(self.stack, 65536)
        self.machine.mem_map(self.sentinel, 4096)
        self.machine.mem_map(self.heap, 65536)
        self.registers = x86_const
        self.unicorn_version = __version__

    def call(self, address, arguments=(), stack_arguments=()):
        if len(arguments) > 4 or len(stack_arguments) > 8:
            raise ValueError("argument list exceeds this helper's bounded Windows x64 ABI")
        registers = self.registers
        pointer = self.stack + 65536 - 264
        self.machine.mem_write(pointer, bytes(256))
        self.machine.mem_write(pointer, struct.pack("<Q", self.sentinel))
        for index, argument in enumerate(stack_arguments):
            self.machine.mem_write(pointer + 40 + index * 8, struct.pack("<Q", argument))
        self.machine.reg_write(registers.UC_X86_REG_RSP, pointer)
        parameter_registers = (registers.UC_X86_REG_RCX, registers.UC_X86_REG_RDX,
                               registers.UC_X86_REG_R8, registers.UC_X86_REG_R9)
        for register, value in zip(parameter_registers, arguments):
            self.machine.reg_write(register, value)
        self.machine.emu_start(address, self.sentinel, timeout=2000000, count=1000000)
        if self.machine.reg_read(registers.UC_X86_REG_RIP) != self.sentinel:
            raise RuntimeError("routine did not return within its emulation budget")
