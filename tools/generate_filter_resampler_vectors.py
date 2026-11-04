"""Export original IIR and settled ModuleUpsampler traces, explicit LE layout."""
import argparse
import hashlib
import json
import struct
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--evidence', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    data = bytearray()

    def words(*values):
        data.extend(struct.pack('<' + 'I' * len(values), *values))

    def floats(*values):
        data.extend(struct.pack('<' + 'f' * len(values), *values))

    cases = json.loads((args.evidence / 'm49-iir-resampler-probes-v1.json').read_text())['cases']
    words(0x5249494D, 1, len(cases))
    for case in cases:
        words(case['factor'], case['descriptor']['latency'], len(case['blocks']))
        for block in case['blocks']:
            words(int(block['reset_before']), len(block['input_bits']))
            for field in ['input_bits', 'up_bits', 'down_bits', 'up_state_bits', 'down_state_bits']:
                words(*block[field])

    cases = json.loads((args.evidence / 'm49-settled-wrapper-probes-v1.json').read_text())['cases']
    words(0x43564F4D, 1, len(cases))
    for case in cases:
        words(case['factor'])
        floats(case['base_rate'])
        words(case['seed'])
        floats(case['initial_cutoff'], case['initial_resonance'])
        words(case['initial_mode'], len(case['blocks']))
        controls = [case['initial_cutoff'], case['initial_resonance'], case['initial_mode'], 0]
        for block in case['blocks']:
            for key, value in block['changes'].items():
                if int(key) < 4:
                    controls[int(key)] = value
            floats(*controls[:2])
            words(int(controls[2]), int(controls[3] > .5), *block['dirty'][:4])
            raw_state = bytes.fromhex(block['raw_state_hex_70_to_17f'])
            seed = struct.unpack_from('<I', raw_state, 4)[0]
            words(int(block['reset_wrapper_and_raw_before']), len(block['audio_bits']), seed)
            for field in ['audio_bits', 'mod_bits', 'output_bits']:
                words(*block[field])
    args.output.write_bytes(data)
    print(f'{len(data)} bytes; SHA256 {hashlib.sha256(data).hexdigest()}')


if __name__ == '__main__':
    main()
