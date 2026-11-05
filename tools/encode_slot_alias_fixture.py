"""Encode frozen original instruction observations; never use candidate outputs."""
import argparse
import hashlib
import json
import pathlib
import struct
import zlib

ORIGINAL_SHA = 'd632c56cc70a69832c65325ae019d9a5491c87d8838285f37be699100a9405d9'
CANONICAL_SHA = 'a2df9bf95778ba4f722b5263be6c3d7a7e02700fe2ab505e9a4f4e4380e06a36'
RESOURCE_LABEL = 0x400020000
TAGS = {'source_owner_select': 1, 'pitch': 2, 'set_word': 3,
        'set_dirty': 4, 'trigger': 5, 'prepare': 6, 'render_scaled': 7}


def encode(source):
    packed = source.read_bytes()
    if hashlib.sha256(packed).hexdigest() != ORIGINAL_SHA:
        raise ValueError('original packed identity')
    canonical = zlib.decompress(packed)
    if hashlib.sha256(canonical).hexdigest() != CANONICAL_SHA:
        raise ValueError('original canonical identity')
    packet = json.loads(canonical)
    if packet['format'] != 'OriginalRawSlotAlias1' or len(packet['operations']) != 206:
        raise ValueError('original profile')
    resource = int(packet['resource_address'], 16)
    initial = packet['initial']
    contexts = {m: bytes.fromhex(initial['models'][str(m)])[0x18:0x20]
                for m in (2, 3)}
    result = bytearray(b'SLOTAL01' + struct.pack('<I', 206))

    def state(model, text):
        data = bytearray.fromhex(text)[:288 if model == 3 else 424]
        if data[0x18:0x20] != contexts[model]:
            raise ValueError('context role changed')
        # Typed explicit record input replaces this immutable pointer role.
        data[0x18:0x20] = bytes(8)
        offsets = (0x30, 0xa8, 0xd0) if model == 3 else (0x30, 0x120)
        for offset in offsets:
            value, = struct.unpack_from('<Q', data, offset)
            displacement = 8192 if model == 3 and offset == 0xd0 else 0
            if value == 0:
                continue
            if value != resource + displacement:
                raise ValueError('resource pointer role')
            struct.pack_into('<Q', data, offset, RESOURCE_LABEL + displacement)
        return data

    def snapshot(value):
        result.extend(state(3, value['models']['3']))
        result.extend(state(2, value['models']['2']))
        role = next((m for m in (2, 3)
                     if value['active'] == packet['models'][str(m)]['interface']), 0)
        if not role and value['active'] != initial['active']:
            raise ValueError('active role')
        active = bytes.fromhex(value['models']['2'])[0x1b0]
        if active not in (0, 1):
            raise ValueError('active flag')
        result.extend(struct.pack('<IBB', value['global_rng'], role, active))

    snapshot(initial)
    previous_memory = bytes.fromhex(initial['memory_hex'])
    if previous_memory != bytes(16384) or initial['global_rng'] != 54321:
        raise ValueError('initial resource/RNG admission')
    result.extend(previous_memory)
    previous = initial
    for operation in packet['operations']:
        if operation['before'] != previous:
            raise ValueError('source continuity')
        args = operation['arguments'] + [0, 0]
        floats = dict(operation.get('float_arguments', []))
        result.extend(struct.pack('<BBIIII', TAGS[operation['operation']],
                                  operation['model'], args[0], args[1],
                                  floats.get(1, 0), floats.get(2, 0)))
        after = operation['after']
        snapshot(after)
        memory = bytes.fromhex(after['memory_hex'])
        if len(memory) != 16384:
            raise ValueError('resource size')
        delta = [i for i in range(8192)
                 if memory[2*i:2*i+2] != previous_memory[2*i:2*i+2]]
        result.extend(struct.pack('<H', len(delta)))
        for index in delta:
            result.extend(struct.pack('<H', index))
            result.extend(memory[2*index:2*index+2])
        if operation['operation'] == 'render_scaled':
            result.extend(struct.pack('<32h', *operation['raw32']))
            result.extend(struct.pack('<32I', *operation['scaled32_bits']))
        previous_memory, previous = memory, after
    raw = bytes(result)
    packed = bytearray(b'KRZP0001' + struct.pack('<II', len(raw), zlib.crc32(raw)))
    position = 0
    while position < len(raw):
        start = position
        zero = raw[position] == 0
        while position < len(raw) and (raw[position] == 0) == zero and position-start < 128:
            position += 1
        packed.append((128 if zero else 0) | (position-start-1))
        if not zero:
            packed.extend(raw[start:position])
    return bytes(packed), raw


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('original', type=pathlib.Path)
    parser.add_argument('output', type=pathlib.Path)
    args = parser.parse_args()
    packed, canonical = encode(args.original)
    args.output.write_bytes(packed)
    print(json.dumps({'packed_bytes': len(packed),
                      'packed_sha256': hashlib.sha256(packed).hexdigest(),
                      'canonical_bytes': len(canonical),
                      'canonical_sha256': hashlib.sha256(canonical).hexdigest()}))


if __name__ == '__main__':
    main()
