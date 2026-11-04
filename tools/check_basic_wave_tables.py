"""Compare generated table bytes with independent original-table SHA-256 values."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dump-program", type=Path)
    parser.add_argument("--dump", type=Path, required=True)
    parser.add_argument("--expected", type=Path, required=True)
    args = parser.parse_args()
    if args.dump_program:
        subprocess.run([str(args.dump_program.resolve()), str(args.dump.resolve())], check=True)
    payload = args.dump.read_bytes()
    if len(payload) != 4 * 32 * 512 * 2:
        raise SystemExit(f"wrong generated table byte count: {len(payload)}")
    expected = json.loads(args.expected.read_text(encoding="utf-8"))
    if len(expected["banks"]) != 128:
        raise SystemExit("reference must cover all 128 bank selections")
    failures = 0
    for index, bank in enumerate(expected["banks"]):
        actual = hashlib.sha256(payload[index * 1024:(index + 1) * 1024]).hexdigest()
        if actual != bank["sha256"]:
            print(f"bank row {bank['row']} index {bank['bank']}: byte hash mismatch")
            failures += 1
    print(f"Original initialized Basic Waves banks: 128 checked, {failures} mismatches")
    if failures:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
