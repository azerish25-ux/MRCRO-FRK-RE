from pathlib import Path
import json,struct,argparse,hashlib
parser=argparse.ArgumentParser();parser.add_argument('--evidence',type=Path,required=True);parser.add_argument('--wave-tables',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--provenance',type=Path,required=True);args=parser.parse_args();out=args.evidence;dst=out/'cpp-candidate'
fs=json.loads((out/'superwave-fixtures.json').read_text())+json.loads((dst/'synthetic-native-fixtures.json').read_text())+json.loads((dst/'control-edge-native-fixtures.json').read_text())+json.loads((dst/'phase-native-fixtures.json').read_text());live=json.loads(args.wave_tables.read_text());kinds=['raw','raw_phase','pitch','initialize','controls','repoint','trigger','interface_controls','interface_initialize','output_wrapper','output_phase','synthetic_raw','constructor','first_prepare','initialize_first_prepare','phase_input','phase_render_trigger'];triggers=0
constructors=json.loads((out/'superwave-constructor-probes.json').read_text())
for key in ['zero_fill_hex','a5_fill_hex']:fs.append({'kind':'constructor','input_hex':constructors[key],'expected_hex':constructors[key]})
with args.output.open('wb') as f:
 f.write(b'SWFX0001');f.write(struct.pack('<128Q',*[int(x,16) for x in live['bank_pointers_va']]));f.write(struct.pack('<I',len(fs)))
 for x in fs:
  flag=triggers%3 if x['kind']=='trigger' else x.get('flag',0);triggers+=x['kind']=='trigger';m=bytes.fromhex(x['input_hex']);e=bytes.fromhex(x['expected_hex']);assert len(m)==344
  f.write(struct.pack('<IffII',kinds.index(x['kind']),x.get('hz',0),x.get('band',0),flag,len(e)));f.write(m);f.write(e)
  if x['kind']=='synthetic_raw':f.write(bytes.fromhex(x['table_hex']))
print(len(fs),'fixtures')

paths=[out/'superwave-fixtures.json',dst/'synthetic-native-fixtures.json',dst/'control-edge-native-fixtures.json',dst/'phase-native-fixtures.json',out/'superwave-constructor-probes.json',args.wave_tables]
report={'processor_sha256':'bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74','schema':'SWFX0001','records':len(fs),'canonical_sha256':hashlib.sha256(args.output.read_bytes()).hexdigest(),'sources':[{'path':p.name,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in paths],'policy':'Only retained original-instruction outputs/input states/table aliases supply expected bytes; opaque native addresses are normalized only by the test representation.'}
args.provenance.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8',newline='\n')
