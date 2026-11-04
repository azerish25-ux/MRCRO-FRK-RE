"""Generate fixtures from original snapshots only; candidate code is never imported."""
import hashlib,json,struct,argparse
from pathlib import Path
parser=argparse.ArgumentParser();parser.add_argument('--evidence',type=Path,required=True);parser.add_argument('--constructor',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--provenance',type=Path,required=True);args=parser.parse_args();here=args.output.parent
inputs=[args.evidence/'m49-normalized-conversion-probes-v1.json',args.evidence/'m49-normalized-dirty-origin-probes-v1.json',args.constructor]
a,b,c=[json.loads(p.read_text()) for p in inputs]
def u(x):return f'0x{int(x):08x}u'
def boolean(x):return 'true' if x else 'false'
def state(s):return '{'+','.join([u(s['normalized_bits']),u(s['physical_bits']),u(s['status']),boolean(s['dirty']),boolean(s['global_dirty'])])+'}'
def fbits(x):return struct.unpack('<I',struct.pack('<f',x))[0]
lines=['#pragma once','#include <array>','#include <cstdint>','namespace original_parameter_reference {',
 'struct State { std::uint32_t normalized_bits, physical_bits, status; bool dirty, aggregate_dirty; };',
 'struct NormalizedCase { std::uint32_t id,input; State before,after_set,after_physical; std::uint32_t normalized_return; State after_normalized; bool has_effective; std::uint32_t effective; };',
 'struct Step { std::uint32_t input; State after_set; };',
 'struct DirtyCase { std::uint32_t id; bool clear_each; State before; std::array<Step,2> steps; };',
 'struct PhysicalCase { std::uint32_t id,input; State before,after_set,after_physical; std::uint32_t normalized_return; State after_normalized; };',
 'struct OriginCase { std::uint32_t id,input; State before,after_set,after_physical; };',
 'struct AggregateCase { std::uint32_t changed_id,unchanged_id,changed_input,unchanged_input; State before_changed,before_unchanged,after_changed,after_unchanged; };',
 'struct Metadata { std::uint32_t id,converter,count,polarity,minimum,maximum; };']
def array(name,typ,rows):
 lines.append(f'inline constexpr std::array<{typ},{len(rows)}> {name}{{{{');lines.extend('  '+row+',' for row in rows);lines.append('}};')
array('normalized_cases','NormalizedCase',['{'+','.join([u(q['id']),u(q['input_normalized_bits']),state(q['before']),state(q['after_set']),state(q['after_physical_get']),u(q['normalized_get_bits']),state(q['after_normalized_get']),boolean(q['prepared_effective_mode_or_bool'] is not None),u(q['prepared_effective_mode_or_bool'] or 0)])+'}' for q in a['normalized_vectors']])
baselines={q['id']:q['before'] for q in a['normalized_vectors']}
array('dirty_cases','DirtyCase',['{'+','.join([u(q['id']),boolean(q['clear_before_each_set']),state(baselines[q['id']]),'{{'+','.join('{'+u(x['normalized_bits'])+','+state(x['state'])+'}' for x in q['sets'])+'}}'])+'}' for q in a['dirty_sequences']])
last_dirty={q['id']:q['sets'][-1]['state'] for q in a['dirty_sequences']}
previous=dict(last_dirty);rows=[]
for q in a['physical_origin_cases']:
 before=previous[q['id']];rows.append('{'+','.join([u(q['id']),u(fbits(q['physical_value'])),state(before),state(q['after_physical_set']),state(q['after_physical_get']),u(q['normalized_get_bits']),state(q['after_normalized_get'])])+'}');previous[q['id']]=q['after_normalized_get']
array('physical_cases','PhysicalCase',rows)
array('origin_cases','OriginCase',['{'+','.join([u(q['id']),u(q['incoming_normalized_bits']),state(q['before']),state(q['after_set']),state(q['after_physical_get'])])+'}' for q in b['physical_origin_then_normalized']])
array('aggregate_cases','AggregateCase',['{'+','.join([u(q['changed_id']),u(q['unchanged_id']),u(q['changed_state']['normalized_bits']),u(q['unchanged_state']['normalized_bits']),state(baselines[q['changed_id']]),state(baselines[q['unchanged_id']]),state(q['changed_state']),state(q['unchanged_state'])])+'}' for q in b['aggregate_dirty_cases']])
array('constructor_states','State',[state(q) for q in c['records']])
array('metadata_rows','Metadata',['{'+','.join(u(q[k]) for k in ['id','converter_type','count_float_bits','polarity_bits','minimum_bits','maximum_bits'])+'}' for q in a['metadata']])
lines.append('} // namespace original_parameter_reference')
args.output.write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n')
provenance={'processor_sha256':c['processor_sha256'],'golden_policy':'Expected snapshots/returns are copied only from original native emulation records. Independent-model fields/match booleans are ignored. Unit prestate restoration uses original baseline/previous snapshots; source clear commands are replayed by the harness. No candidate-derived expected values.','sources':[{'path':p.name,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in inputs],'trace_cases':len(a['normalized_vectors'])+sum(len(q['sets']) for q in a['dirty_sequences'])+len(a['physical_origin_cases'])+len(b['physical_origin_then_normalized'])+len(b['aggregate_dirty_cases']),'constructor_states':len(c['records']),'metadata_rows':len(a['metadata']),'generated_header_sha256':hashlib.sha256(args.output.read_bytes()).hexdigest()}
args.provenance.write_text(json.dumps(provenance,indent=2));print(json.dumps({'original_trace_cases':provenance['trace_cases'],'constructor_states':provenance['constructor_states']}))
