"""Project frozen original Master observations into typed audio-stage gold."""
import argparse
import hashlib
import json
import pathlib
import struct
import zlib

PACKED_SHA = 'bf5e9b184f7a18d3e96699ccfb3dc8014497e5da40e2d78b1946448ab12b90d3'
CANONICAL_SHA = 'b9ed8d67c08e2b4c05d062c935bce4b9b961ddfe7975553fc603e7727312c058'


def encode(source):
    packed = source.read_bytes()
    if hashlib.sha256(packed).hexdigest() != PACKED_SHA:
        raise ValueError('original packed identity')
    raw = zlib.decompress(packed)
    if hashlib.sha256(raw).hexdigest() != CANONICAL_SHA:
        raise ValueError('original canonical identity')
    original = json.loads(raw)
    if len(original['cases']) != 4:
        raise ValueError('case count')
    result = bytearray(b'MOCOLD01' + struct.pack('<I', 4))

    def words(values):
        result.extend(struct.pack('<' + 'I' * len(values), *values))

    def snapshot(value):
        header = bytes.fromhex(value['header_b8_hex'])
        if len(header) != 184:
            raise ValueError('native header size')
        word = lambda offset: struct.unpack_from('<I', header, offset)[0]
        if header[0x80] not in (0, 1):
            raise ValueError('mono flag')
        words([word(0x70), word(0x7c), header[0x80], word(0xb4)])
        for offsets in [(0x30, 0x34, 0x38, 0x3c), (0x20, 0x24, 0x28, 0x2c),
                        (0x40, 0x44, 0x48, 0x58, 0x5c, 0x60),
                        (0x90, 0x94, 0xa0, 0xa4)]:
            words([word(offset) for offset in offsets])
        if len(value['gain_buffer16']) != 16 or [len(row) for row in value['rings2x145']] != [145, 145]:
            raise ValueError('workspace dimensions')
        words(value['gain_buffer16'])
        words([x for row in value['rings2x145'] for x in row])

    for case in original['cases']:
        ready = {'clean-mini-cold-v1.pklz': 0, 'clean-mini-ready-v1.pklz': 1}[case['checkpoint']]
        if type(case['inplace']) is not bool or len(case['blocks']) != 28:
            raise ValueError('case profile')
        result.extend(bytes([ready, int(case['inplace'])]))
        snapshot(case['initial'])
        words([28])
        for block in case['blocks']:
            dirty = block['supplied_dirty']
            if len(dirty) != 3 or any(x not in (0, 1) for x in dirty) or type(block['reset']) is not bool:
                raise ValueError('control flags')
            result.extend(bytes(dirty + [int(block['reset'])]))
            snapshot(block['prepared'])
            for name in ['input_bits', 'output_bits', 'input_after_bits']:
                if [len(row) for row in block[name]] != [16, 16]:
                    raise ValueError('audio dimensions')
                words([x for row in block[name] for x in row])
            snapshot(block['after'])
    return bytes(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('original', type=pathlib.Path)
    parser.add_argument('output', type=pathlib.Path)
    args = parser.parse_args()
    data = encode(args.original)
    args.output.write_bytes(data)
    print(json.dumps({'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}))


if __name__ == '__main__':
    main()
