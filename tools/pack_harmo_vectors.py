"""Losslessly pack canonical original Harmo observations; no expected-value math."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import zlib
from pack_superwave_vectors import pack, unpack


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input",type=Path,required=True)
    parser.add_argument("--output",type=Path,required=True)
    parser.add_argument("--provenance",type=Path,required=True)
    args=parser.parse_args()
    data=args.input.read_bytes()
    if not 12<=len(data)<=16*1024*1024 or data[:8]!=b"HARMO002":
        raise ValueError("Canonical Harmo fixture identity/size")
    packets=pack(data)
    if unpack(packets,len(data))!=data:raise ValueError("Roundtrip mismatch")
    payload=b"HARMOR01"+struct.pack("<II",len(data),zlib.crc32(data))+packets
    args.output.write_bytes(payload)
    report={"canonical_bytes":len(data),"packed_bytes":len(payload),
            "canonical_sha256":hashlib.sha256(data).hexdigest(),
            "packed_sha256":hashlib.sha256(payload).hexdigest(),
            "canonical_crc32":zlib.crc32(data),"byte_identical_roundtrip":True}
    args.provenance.write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")
    print(json.dumps(report))


if __name__=="__main__":main()
