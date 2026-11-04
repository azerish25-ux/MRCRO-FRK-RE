"""Initialize the exact processor's emulated mathematical tables from verified evidence.

Only table data in the known .data range is supplied. Original instructions are
unchanged. Every table was independently checked against live bytes, initializer
instruction emulation and reconstructed formulas before use as fixture input.
"""
import hashlib
import json
from pathlib import Path

from native_reference import PROCESSOR_SHA256


def populate_verified_tables(reference, evidence_directory):
    directory = Path(evidence_directory)
    identity = json.loads((directory / "live-tables.json").read_text(encoding="utf-8"))
    verification = json.loads((directory / "emulated-table-verification.json").read_text(encoding="utf-8"))
    if identity["module_sha256"] != PROCESSOR_SHA256 or len(verification["cases"]) != 34:
        raise ValueError("table evidence does not match the identified processor and full table set")
    hashes = []
    for case in verification["cases"]:
        address = int(case["table"], 16)
        if not 0x18059C000 <= address <= 0x1805BA280 - 1024:
            raise ValueError("table address is outside the identified data range")
        payload = (directory / f"table-{address:09x}.bin").read_bytes()
        digest = hashlib.sha256(payload).hexdigest()
        if (len(payload) != 1024 or digest != case["generated_sha256"] or
                not case["matches_live"] or case["mismatched_shorts"] != 0):
            raise ValueError("table input failed independent evidence validation")
        reference.machine.mem_write(address, payload)
        hashes.append({"address": hex(address), "sha256": digest})
    return hashes
