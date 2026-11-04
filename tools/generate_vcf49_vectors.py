"""Export bounded original model49 traces as explicit little-endian test data."""
import argparse
import hashlib
import json
import struct
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--evidence', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    data = bytearray()

    def words(*values):
        data.extend(struct.pack('<' + 'I' * len(values), *values))

    def controls(values, dirty, first, force):
        data.extend(struct.pack('<ff', values[0], values[1]))
        words(int(values[2]), int(values[3] > 0.5), *dirty[:4], int(first), int(force))

    cases = []
    for filename in ['m49-raw-probes-v2.json', 'm49-lifecycle-probes-v3.json']:
        cases.extend(json.loads((args.evidence / filename).read_text())['cases'])
    words(0x3439464D, 1, len(cases))
    samples = blocks = 0
    for case in cases:
        lifecycle = 'initial_mode' in case
        state = bytes.fromhex(case['initial_state_hex_70_to_17f'])
        seed = struct.unpack_from('<I', state, 4)[0]
        initial = ([case['initial_cutoff'], case['initial_resonance'], case['initial_mode'], 0]
                   if lifecycle else [case['base_cutoff'], case['resonance'], case['mode'], case['noise']])
        data.extend(struct.pack('<f', case['rate']))
        words(seed)
        controls(initial, [1]*4, True, True)
        words(len(case['blocks']))
        for block in case['blocks']:
            current = [block['controls'][str(i)] for i in range(4)] if lifecycle else initial
            controls(current, block.get('dirty', [0]*4), block.get('first', False), block.get('force', False))
            state = bytes.fromhex(block['state_hex_70_to_17f'])
            seed = struct.unpack_from('<I', state, 4)[0]
            count = len(block['audio_input_bits'])
            words(int(block.get('reset_after_prepare', False)), count, seed)
            for field in ['audio_input_bits', 'modulation_input_bits', 'output_bits']:
                assert len(block[field]) == count
                words(*block[field])
            samples += count
            blocks += 1
    args.output.write_bytes(data)
    print(f'{len(cases)} cases, {blocks} blocks, {samples} samples; {len(data)} bytes; SHA256 {hashlib.sha256(data).hexdigest()}')


if __name__ == '__main__':
    main()
