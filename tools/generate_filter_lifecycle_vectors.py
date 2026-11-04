"""Serialize original cold/controller/worker/pending traces, not port results."""
import argparse
import hashlib
import json
import struct
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--evidence', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--provenance', type=Path)
    args = parser.parse_args()
    commands = []
    sources = {}
    counts = {'audio_samples':0,'snapshots':0,'parameter_snapshots':0,'suspended_checks':0}
    def words(*v): return struct.pack('<'+'I'*len(v), *v)
    def floats(*v): return struct.pack('<'+'f'*len(v), *v)
    def read(name):
        payload = (args.evidence/name).read_bytes()
        sources[name] = hashlib.sha256(payload).hexdigest()
        return json.loads(payload)
    def expected(source=None, factor=None, seed=None):
        values = [0]*12
        mask = 0
        def put(index, value):
            nonlocal mask
            values[index] = int(value)
            mask |= 1 << index
        if source is not None:
            for index, key in [(0,'ready'),(1,'fade_current_bits'),(2,'fade_target_bits'),(3,'resume_flag')]:
                if key in source: put(index,source[key])
            if source.get('factor') is not None: put(4,source['factor'])
            elif factor is not None: put(4,factor)
            if source.get('filter_type') is not None: put(5,source['filter_type'])
            elif source.get('type') is not None: put(5,source['type'])
            else: put(5,0)
            if 'latency_fields' in source: put(6,source['latency_fields'][1])
            if 'latency_flag' in source: put(7,source['latency_flag'])
            if 'outer_first_force' in source:
                put(8,source['outer_first_force'][0]); put(9,source['outer_first_force'][1])
            if 'raw_prng_seed' in source: put(10,source['raw_prng_seed'])
            elif 'prng_seed' in source: put(10,source['prng_seed'])
            elif seed is not None: put(10,seed)
            if source.get('request_write_read') is not None:
                request = source['request_write_read']; completion = source['completion_write_read']
                put(11,1 if request[0]!=request[1] else 3 if completion[0]!=completion[1] else 0)
        return words(mask,*values)
    def add(op, payload=b'', state=None, factor=None, seed=None):
        rows = [] if state is None else state.get('parameters',[])
        parameter_data = words(len(rows))
        for row in rows:
            parameter_data += words(row['id'],row['normalized_bits'],row['physical_bits'],row['status'],row['dirty'],row['global_dirty'])
        commands.append(bytes([op])+payload+expected(state,factor,seed)+parameter_data)
        if op == 5: counts['audio_samples'] += 32
        if op == 9: counts['suspended_checks'] += 1
        if state is not None: counts['snapshots'] += 1
        counts['parameter_snapshots'] += len(rows)

    for case in read('m49-cold-start-probes-v2.json')['cases']:
        seed = case['after_constructor']['raw_prng_seed']
        add(0,words(seed,1),case['after_constructor'],factor=0)
        add(3,floats(case['base_rate']),case['after_public_init'])
        if case['change_factor_index_before_first_prepare']: add(2,words(6)+floats(1.0))
        add(4,words(case['context_plus50']),case['after_first_prepare'])
        add(5,words(*case['first_audio_bits'],*case['first_mod_bits'],*case['first_output_bits']),case['after_first_render'])
        if case['after_explicit_worker_drain'] is not None:
            add(7,state=case['after_explicit_worker_drain'])
            add(4,words(0),case['after_explicit_completion_drain'])

    cases = [('m49-transition-controller-probes-v2.json',4),
             ('m49-transition-factor1-probes-v1.json',1),
             ('m49-transition-factor3-probes-v1.json',3),
             ('m49-transition-pending-probes-v1.json',4)]
    for filename,target in cases:
        trace = read(filename)
        adaptive = target in (1,3)
        add(0,words(1,int(adaptive)))
        for parameter,value in [(0,.45),(1,.99),(2,0.0),(3,1.0)]: add(1,words(parameter)+floats(value))
        # Equivalent stable old2x resource for the source's explicit initial2x.
        # Adaptive cases select4>>1 through physical index1 before public Init.
        if adaptive: add(2,words(6)+floats(1.0))
        add(3,floats(96000.0)); add(6); add(4,words(0))
        previous = 'settled_old'
        last_seed = 1
        for block in trace['audio_blocks']:
            phase = block['phase']
            if phase == 'fade_out_old' and previous != phase:
                index = {4:1,1:0,3:2}[target]
                add(2,words(6)+floats(float(index)))
                add(4,words(0),trace['after_request'],2,last_seed)
            if phase == 'fade_in_new' and previous != phase:
                add(8,state=trace['after_fade_out'],factor=2,seed=last_seed)
                assert trace['suspended_output_unchanged'] and trace['suspended_prng_unchanged']
                add(9)
                if 'pending_controls' in trace:
                    pending = trace['pending_controls']
                    for parameter,value in pending['enqueue_order']: add(1,words(parameter)+floats(value))
                    for parameter,value in enumerate(pending['normalized_after_enqueue_bits']): add(11,words(parameter,value))
                    pairs = pending['stored_pair_words']; add(10,words(len(pairs)//2,*pairs))
                add(7,state=trace['after_worker'],factor=target,seed=last_seed)
                add(4,words(0),trace['after_completion'],target,last_seed)
                add(10,words(0))
                if 'pending_controls' in trace:
                    for parameter,value in enumerate(trace['pending_controls']['normalized_after_replay_bits']): add(11,words(parameter,value))
            state = bytes.fromhex(block['raw_state_hex_70_to_17f'])
            last_seed = struct.unpack_from('<I',state,4)[0]
            factor = target if phase == 'fade_in_new' else 2
            add(5,words(*block['audio_bits'],*block['mod_bits'],*block['output_bits']),block['control_state'],factor,last_seed)
            if phase == 'settled_old': add(6)
            previous = phase
        add(8,state=trace['final'],factor=target,seed=last_seed)
    for case in read('m49-cross-kernel-transition-probes-v1.json')['cases']:
        add(0,words(case['seed'],int(case['adaptive'])))
        for parameter,value in case['initial_normalized_commands']: add(1,words(parameter)+floats(value))
        add(3,floats(case['base_rate']),case['after_public_init'])
        previous = 'settled_old'
        for block in case['blocks']:
            phase = block['phase']
            if phase == 'fade_out_old' and previous != phase:
                for parameter,value in case['request_normalized_commands']: add(1,words(parameter)+floats(value))
                add(8,state=case['before_request'])
                add(4,words(0),case['after_request'])
            if phase == 'fade_in_new' and previous != phase:
                add(8,state=case['after_fade_out'])
                assert case['gap_output_preserved'] and case['gap_seed_preserved']
                add(4,words(0)); add(9)
                add(7,state=case['after_worker'])
                add(4,words(0),case['after_completion'])
            add(4,words(0))
            add(5,words(*block['audio_bits'],*block['mod_bits'],*block['output_bits']))
            add(6,state=block['controller'])
            previous = phase
        add(8,state=case['final'])
    payload = words(0x4c46564d,2,len(commands))+b''.join(commands)
    args.output.write_bytes(payload)
    if args.provenance:
        report = {'processor_sha256':'bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74',
                  'schema':'MVFL2','commands':len(commands),'fixture_bytes':len(payload),
                  'fixture_sha256':hashlib.sha256(payload).hexdigest(),**counts,
                  'sources':[{'path':name,'sha256':value} for name,value in sources.items()],
                  'policy':'Only original captured input/output/control/cache/seed records supply expectations. Windows ownership/thread shells remain modeled, worker schedule explicit. Equivalent stable old2x for adaptive1/3 cases uses constructor index1; this does not replace source audio expectations. Portable allocation-failure recovery is separate from native gold.'}
        args.provenance.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(f'{len(commands)} commands, {len(payload)} bytes; SHA256 {hashlib.sha256(payload).hexdigest()}')


if __name__ == '__main__': main()
