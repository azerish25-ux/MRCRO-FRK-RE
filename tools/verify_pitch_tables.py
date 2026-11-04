"""Compare upstream MIT pitch tables with this exact installed processor."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

from scan_pe_strings import pe_layout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("processor", type=Path)
    parser.add_argument("upstream_units", type=Path)
    args = parser.parse_args()
    data = args.processor.read_bytes()
    expected = "bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74"
    if hashlib.sha256(data).hexdigest() != expected:
        parser.error("unexpected processor identity")
    image_base, sections = pe_layout(data)
    source = args.upstream_units.read_text(encoding="utf-8")
    results = []
    for name, address in (("lut_pitch_ratio_high", 0x18046C3A0),
                          ("lut_pitch_ratio_low", 0x18046C7B0)):
        match = re.search(r"const float " + name + r"\[\]\s*=\s*\{(.*?)\};", source, re.S)
        if not match:
            raise RuntimeError(f"upstream table missing: {name}")
        values = [float(token.strip().rstrip("f")) for token in match[1].split(",") if token.strip()]
        upstream_bytes = struct.pack("<" + "f" * len(values), *values)
        section = next(s for s in sections if s["rva"] <= address - image_base
                       and address - image_base + len(upstream_bytes) <= s["rva"] + s["size"])
        offset = section["file_offset"] + address - image_base - section["rva"]
        observed = data[offset:offset + len(upstream_bytes)]
        results.append({"name": name, "entries": len(values), "address": hex(address),
                        "bytes_identical": upstream_bytes == observed,
                        "upstream_sha256": hashlib.sha256(upstream_bytes).hexdigest(),
                        "installed_sha256": hashlib.sha256(observed).hexdigest()})
    print(json.dumps({"processor_sha256": expected, "tables": results}, indent=2))
    if not all(result["bytes_identical"] for result in results):
        raise SystemExit(1)


if __name__ == "__main__":
    main()
