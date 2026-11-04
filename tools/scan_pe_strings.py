"""Locate ASCII string candidates in a PE image without loading its code.

Addresses map file-backed bytes only. A string is a search seed, not evidence
that the associated feature or library executes.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct


def pe_layout(data):
    if data[:2] != b"MZ" or len(data) < 64:
        raise ValueError("not a DOS/PE image")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe + 4] != b"PE\0\0":
        raise ValueError("PE signature missing")
    sections_count = struct.unpack_from("<H", data, pe + 6)[0]
    optional_size = struct.unpack_from("<H", data, pe + 20)[0]
    optional = pe + 24
    magic = struct.unpack_from("<H", data, optional)[0]
    if magic == 0x20B:
        image_base = struct.unpack_from("<Q", data, optional + 24)[0]
    elif magic == 0x10B:
        image_base = struct.unpack_from("<I", data, optional + 28)[0]
    else:
        raise ValueError("unsupported optional header")
    sections = []
    start = optional + optional_size
    if start + sections_count * 40 > len(data):
        raise ValueError("section table exceeds input")
    for index in range(sections_count):
        offset = start + index * 40
        name = data[offset:offset + 8].split(b"\0")[0].decode("ascii", errors="replace")
        _, rva, size, file_offset = struct.unpack_from("<IIII", data, offset + 8)
        sections.append({"name": name, "rva": rva, "size": size, "file_offset": file_offset})
    return image_base, sections


def scan(path, pattern):
    data = path.read_bytes()
    image_base, sections = pe_layout(data)
    matches = []
    query = re.compile(pattern, re.IGNORECASE)
    for match in re.finditer(rb"[\x20-\x7e]{6,}", data):
        value = match.group().decode("ascii")
        if not query.search(value):
            continue
        section = next((s for s in sections if s["file_offset"] <= match.start()
                        and match.end() <= s["file_offset"] + s["size"]), None)
        address = image_base + section["rva"] + match.start() - section["file_offset"] if section else None
        matches.append({"file_offset": match.start(), "address": hex(address) if address else None,
                        "section": section["name"] if section else None, "value": value})
    return {"sha256": hashlib.sha256(data).hexdigest(), "path": str(path.resolve()),
            "pattern": pattern, "encoding_scope": "ASCII runs of at least six bytes",
            "sections": sections, "matches": matches}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", type=Path)
    parser.add_argument("--pattern", required=True)
    args = parser.parse_args()
    print(json.dumps(scan(args.path, args.pattern), indent=2))


if __name__ == "__main__":
    main()
