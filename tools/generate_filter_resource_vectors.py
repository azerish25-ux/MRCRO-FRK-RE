"""Export retained original FIR/fade/selector expectations, explicit LE data."""
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
    def words(*values): data.extend(struct.pack('<' + 'I' * len(values), *values))
    def floats(*values): data.extend(struct.pack('<' + 'f' * len(values), *values))
    def read(name): return json.loads((args.evidence / name).read_text())

    words(0x5246534D, 1)
    cases = read('m49-fir-audio-probes-v1.json')['cases']
    resources = {c['factor']: c for c in read('m49-original-fir-resource-construction-v1.json')['cases']}
    words(len(cases))
    for case in cases:
        latency = resources[case['factor']]['input_resamplers'][0]['latency']
        words(case['factor'], latency, len(case['blocks']))
        for block in case['blocks']:
            words(int(block['reset_before']), len(block['input_bits']))
            for key in ['input_bits', 'up_bits', 'down_bits']: words(*block[key])

    cases = read('m49-iir-factor3-probes-v1.json')['cases']
    words(len(cases))
    for case in cases:
        words(case['factor'], case['descriptor']['latency'], case['descriptor']['sections'] * 2, len(case['blocks']))
        for block in case['blocks']:
            words(int(block['reset_before']), len(block['input_bits']))
            for key in ['input_bits', 'up_bits', 'down_bits', 'up_state_bits', 'down_state_bits']: words(*block[key])

    cases = read('m49-fade-vector-probes-v1.json')['cases']
    words(len(cases))
    for case in cases:
        floats(case['rate']); words(case['time_bits'])
        floats(case['initial_current'], case['target']); words(case['step_bits'], len(case['blocks']))
        for block in case['blocks']: words(*block['output_bits'], block['current_bits'])

    cases = read('m49-factor-selector-probes-v1.json')['cases']
    words(len(cases))
    for case in cases:
        words(case['factor_index'], int(case['adaptive'])); floats(case['rate']); words(case['actual'])
    cases = read('m49-fir-settled-wrapper-probes-v1.json')['cases']
    words(len(cases))
    for case in cases:
        words(case['factor']); floats(case['base_rate']); words(case['seed'])
        floats(case['initial_cutoff'], case['initial_resonance']); words(case['initial_mode'],len(case['blocks']))
        current = [case['initial_cutoff'],case['initial_resonance'],case['initial_mode'],0]
        for block in case['blocks']:
            dirty = [0]*4
            for key,value in block['normalized_changes'].items():
                index = int(key)
                current[index] = int(value*2) if index == 2 else value
                dirty[index] = 1
            floats(*current[:2]); words(int(current[2]),int(current[3]>.5),*dirty)
            state = bytes.fromhex(block['raw_state_hex_70_to_17f'])
            words(int(block['reset_wrapper_and_raw_before']),len(block['audio_bits']),struct.unpack_from('<I',state,4)[0])
            for key in ['audio_bits','mod_bits','output_bits']: words(*block[key])
    args.output.write_bytes(data)
    print(f'{len(data)} bytes; SHA256 {hashlib.sha256(data).hexdigest()}')


if __name__ == '__main__': main()
