"""Export original-instruction impulse/noise traces; raw evidence stays local."""
import argparse
import json
import struct
from pathlib import Path


def word(value):
    return struct.unpack('<I', struct.pack('<f', value))[0]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--evidence', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    lines = ['// Original instruction traces: processor SHA256 bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74.',
             '#pragma once', '#include <array>', '#include <cstdint>', '#include <span>',
             'struct VcfExtraBlock { bool noise; std::uint32_t seed_after; std::array<std::uint32_t,32> input,output; };',
             'struct VcfExtraScenario { std::uint32_t rate,seed,cutoff,resonance; unsigned mode; bool noise; std::span<const VcfExtraBlock> blocks; };']
    cases = []
    for filename in ['raw-vcf-impulse-probes.json', 'raw-vcf-noise-probes.json']:
        document = json.loads((args.evidence / filename).read_text())
        for case in document['cases']:
            index = len(cases)
            noise = 'blocks' in case
            blocks = case.get('blocks')
            if not noise:
                blocks = [{'noise_enabled': False, 'seed_after': 1,
                           'input_bits': [word(1.0) if b == i == 0 else 0 for i in range(32)],
                           'output_bits': output} for b, output in enumerate(case['output_bits'])]
            lines.append(f'inline constexpr VcfExtraBlock kVcfExtraBlocks{index}[] = {{')
            for block in blocks:
                lines.append('    {%s, 0x%08xu, {' % (str(block['noise_enabled']).lower(), block['seed_after']))
                for field in ['input_bits', 'output_bits']:
                    for start in range(0, 32, 8):
                        lines.append('        ' + ', '.join('0x%08xu' % v for v in block[field][start:start+8]) + ',')
                    lines.append('    }, {' if field == 'input_bits' else '    }},')
            lines.append('};')
            cases.append('    {%s, %s, %s, %s, %d, %s, kVcfExtraBlocks%d},' %
                         tuple(['0x%08xu' % v for v in [word(case['rate']), case.get('seed', 1), word(case['cutoff']), word(case['resonance'])]] +
                               [case['mode'], str(noise).lower(), index]))
    lines.extend(['inline constexpr VcfExtraScenario kVcfExtraScenarios[] = {', *cases, '};', ''])
    args.output.write_text('\n'.join(lines), encoding='utf-8', newline='\n')


if __name__ == '__main__':
    main()
