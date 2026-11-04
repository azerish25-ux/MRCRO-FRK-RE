import pathlib,json,struct,hashlib,argparse
parser=argparse.ArgumentParser();parser.add_argument('--evidence',type=pathlib.Path,required=True);parser.add_argument('--output-dir',type=pathlib.Path,required=True);args=parser.parse_args();out=args.output_dir;src=args.evidence;out.mkdir(parents=True,exist_ok=True)
a=json.loads((src/'fx-seven-lane-sum-probes.json').read_text());cold=json.loads((src/'fx-cold-default-store-probes.json').read_text());data=bytearray(b'FXMIX001')
def integer(v,n=4):data.extend(int(v).to_bytes(n,'little'))
integer(len(a['cases']))
for c in a['cases']:
 integer(c['external_left_present'],1);integer(c['external_right_present'],1)
 for lane in c['lane_bits']:
  for v in lane:integer(v)
 for v in c['external_bits']+c['actual_bits']:integer(v)
integer(len(cold['cases']))
for c in cold['cases']:
 data.extend(bytes.fromhex(c['history_14_floats_hex']));data.extend(bytes.fromhex(c['width_float_hex']));data.extend(bytes.fromhex(c['flags_7fb4_7fb6'])[:1]);data.extend(bytes.fromhex(c['pattern_ushort_hex']))
p=out/'original-fx-mix-fixtures.bin';p.write_bytes(data)
manifest={'fixture':p.name,'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'processor_sha256':a['processor_sha256'],'sources':[{'path':f,'sha256':hashlib.sha256((src/f).read_bytes()).hexdigest()} for f in ['fx-seven-lane-sum-probes.json','fx-cold-default-store-probes.json']],'sum_cases':len(a['cases']),'sum_float_values':len(a['cases'])*32,'cold_cases':len(cold['cases']),'expected_origin':'Only actual bits/bytes from retained original-instruction probes; candidate is neither imported nor executed.','layout':'FXMIX001; u32 sumcount; each u8L/u8R,224 lane u32 bits,32 external u32 bits,32 original expected u32 bits; u32 coldcount; each14float history, float width, u8 disable byte, u16 pattern. All integers little-endian.'}
(out/'mix-fixture-manifest.json').write_text(json.dumps(manifest,indent=2));print('serialized',len(data),'original bytes')
