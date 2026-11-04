"""Lossless bounded zero-run packets for the canonical original SWFX fixture."""
import argparse
import hashlib
import json
import struct
import zlib
from pathlib import Path


def pack(data):
    result = bytearray()
    index = 0
    while index < len(data):
        count = 0
        while index+count < len(data) and count < 128 and data[index+count] == 0: count += 1
        if count >= 3:
            result.append(128+count-1); index += count; continue
        start = index
        index += 1
        while index < len(data) and index-start < 128:
            if data[index:index+3] == b'\0\0\0': break
            index += 1
        result.append(index-start-1); result.extend(data[start:index])
    return bytes(result)


def unpack(data, size):
    result = bytearray()
    index = 0
    while index < len(data):
        tag = data[index]; index += 1; count = (tag & 127)+1
        if count > size-len(result): raise ValueError('decoded length overflow')
        if tag & 128: result.extend(bytes(count))
        else:
            if count > len(data)-index: raise ValueError('truncated literal')
            result.extend(data[index:index+count]); index += count
    if len(result) != size: raise ValueError('wrong decoded length')
    return bytes(result)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--provenance', type=Path, required=True)
    args = parser.parse_args()
    original = args.input.read_bytes()
    if original[:8] != b'SWFX0001': raise ValueError('wrong canonical fixture')
    packets = pack(original)
    assert unpack(packets,len(original)) == original
    payload = b'SWZP0001'+struct.pack('<II',len(original),zlib.crc32(original))+packets
    args.output.write_bytes(payload)
    report = {'schema':'SWZP0001','canonical_schema':'SWFX0001',
              'canonical_bytes':len(original),'fixture_bytes':len(payload),
              'canonical_sha256':hashlib.sha256(original).hexdigest(),
              'fixture_sha256':hashlib.sha256(payload).hexdigest(),
              'canonical_crc32':zlib.crc32(original),'byte_identical_roundtrip':True,
              'records':struct.unpack_from('<I',original,8+128*8)[0],
              'policy':'Every original canonical byte is retained; no candidate-derived expectations or dropped state fields. Zero packets/literals max128; decoder checks size, CRC32, framing and trailing canonical data.'}
    args.provenance.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps(report))


if __name__ == '__main__': main()
