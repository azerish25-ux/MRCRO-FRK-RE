"""Pack retained original-instruction Bass observations, never candidate output."""
import argparse
import json
from pathlib import Path
import struct


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("observations", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    rows = json.loads(args.observations.read_text(encoding="utf-8"))
    kinds = ["raw", "scaled", "controls", "interface_controls", "initialize",
             "interface_initialize", "pitch", "phase", "trigger", "constructor"]
    assert len(rows) == 776
    data = bytearray(b"BASSFX01" + struct.pack("<I", len(rows)))
    for row in rows:
        before = bytes.fromhex(row["input_hex"])
        expected = bytes.fromhex(row["state_hex"])
        audio = bytes.fromhex(row["output_hex"])
        kind = kinds.index(row["kind"])
        assert len(before) == len(expected) == 128
        assert len(audio) == (128 if kind < 2 else 0)
        data.extend(struct.pack("<IfII", kind, row.get("arg", 0),
                                row.get("flag", 0), len(audio)))
        data.extend(before + expected + audio)
    args.output.write_bytes(data)
    print(f"{len(rows)} fixtures, {len(data)} bytes")


if __name__ == "__main__":
    main()
