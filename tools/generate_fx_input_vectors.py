"""Serialize original VM/SystemCRT observations; no candidate/scalar model imports."""
import pathlib,json,struct,hashlib,argparse
parser=argparse.ArgumentParser();parser.add_argument('--evidence',type=pathlib.Path,required=True);parser.add_argument('--output-dir',type=pathlib.Path,required=True);args=parser.parse_args();ROOT=args.output_dir;E=args.evidence;ROOT.mkdir(parents=True,exist_ok=True)
prefix=E/'fx-vca-pan-prefix-probes-v2.json';control=E/'voice-vca-controller-probes-v2.json';norm=E/'vca-normalization-probes.json';a=json.loads(prefix.read_text());b=json.loads(control.read_text());c=json.loads(norm.read_text());data=bytearray(b'FXVCA001')
def put(fmt,*v):data.extend(struct.pack('<'+fmt,*v))
def floats(xs):put(str(len(xs))+'f',*xs)
def words(xs):put(str(len(xs))+'h',*xs)
put('I',len(a['cases']))
for case in a['cases']:
 put('BBfI',case['mode_fixture'],case['disable_spread_flag_fixture'],case['width_fixture'],case['consecutive_block'])
 put('112I',*case['input_mono_bits']);floats(case['levels']);floats(case['pans']);floats([v for pair in case['previous_lr_gains'] for v in pair]);floats([v for pair in case['target_lr_gains'] for v in pair]);put('224I',*case['actual_stereo_voice_bits'])
put('I',len(b['cases']))
for case in b['cases']:
 put('hBhhhB',case['input_adsr_q15'],case['velocity'],case['velo_env_q14'],case['velo_vca_q14'],case['postmatrix_effective_vca_fixture'],case['para_flag']);words(case['source_env_q15']);words(case['actual_vca_q14'])
put('I',len(c['cases']))
for case in c['cases']:put('hHI',case['q14_input'],case['original_ushort'],case['original_published_normalized_bits'])
(ROOT/'original-fx-input-fixtures.bin').write_bytes(data)
files=[prefix,control,norm,E/'probe-fx-vca-prefix-v2.py',E/'probe-vca-controller-v2.py',E/'probe-vca-normalize.py']
manifest={'format':'FXVCA001','original_processor_sha256':a['processor_sha256'],'fixture_bytes':len(data),'fixture_sha256':hashlib.sha256(data).hexdigest(),'prefix_cases':len(a['cases']),'stereo_sample_checks':len(a['cases'])*224,'q14_controller_cases':len(b['cases']),'q14_lane_words_each_kind':len(b['cases'])*12,'normalization_cases':len(c['cases']),'original_sources':[{'path':f.name,'sha256':hashlib.sha256(f.read_bytes()).hexdigest()} for f in files],'scope':'Seven mono16-sample lanes only. Full original prefix to30f4e3, explicit prior gains/configuration; no eighth input, external stereo override, summing/effects or original startup-history claim. Q14v2 probes inject legal postmatrix gain edge values at2e3152. Expected behavioral outputs are retained original VM/SystemCRT observations; candidate/scalar code never imported.'};(ROOT/'fixture-manifest.json').write_text(json.dumps(manifest,indent=2));print(manifest['fixture_bytes'],'bytes',manifest['prefix_cases'],'prefixcases',manifest['q14_controller_cases'],'Q14cases')
# These small immutable arrays are recovered source constants, not expected outputs.
const=a['constants'];lines=['#pragma once','#include <array>','#include <bit>','#include <cstdint>','namespace mfrk::fx_input::detail {']
for name,key in [('quarter','quarter_pi_bits'),('three_quarter','threequarter_pi_bits'),('calibration','gain_calibration_bits')]:lines.append(f'inline constexpr float {name} = std::bit_cast<float>(std::uint32_t{{{const[key]}u}});')
for i,pattern in enumerate(const['pan_patterns']):
 vals=struct.unpack('<7I',struct.pack('<7f',*pattern));lines.append('inline constexpr std::array<float,7> pattern'+str(i)+'{');lines.extend('  std::bit_cast<float>(std::uint32_t{'+str(v)+'u}),' for v in vals);lines.append('};')
lines.append('}');(ROOT/'source_constants.hpp').write_text('\n'.join(lines)+'\n')
