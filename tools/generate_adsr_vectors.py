"""Serialize retained original expectations; never imports candidate/scalar code."""
import pathlib,json,struct,hashlib,argparse
parser=argparse.ArgumentParser();parser.add_argument('--evidence',type=pathlib.Path,required=True);parser.add_argument('--output-dir',type=pathlib.Path,required=True);args=parser.parse_args();ROOT=args.output_dir;E=args.evidence;ROOT.mkdir(parents=True,exist_ok=True)
FIELDS=['phase','mode','value_q31','current_q15','output_q15','retrigger_ticks','hold','pending_release','legato_flag','trigger','time_a','time_d','time_r','sustain','variant_a','variant_d','variant_r','aux_d','aux_r','const_d0','const_d1','const_d2','const_r0','const_r1','const_r2','dynamic_d','dynamic_r','coef_a','add_a','coef_d','add_d','coef_r','add_r']
CPP=['phase','mode','accumulator','current','output','retrigger_ticks','hold','pending_release','legato','trigger','attack_time','decay_time','release_time','sustain','attack_variant','decay_variant','release_variant','decay_aux','release_aux','decay_constants[0]','decay_constants[1]','decay_constants[2]','release_constants[0]','release_constants[1]','release_constants[2]','decay_dynamic','release_dynamic','attack_coefficient','attack_additive','decay_coefficient','decay_additive','release_coefficient','release_additive']
SOURCES=['adsr-constructor-emulated-crt.bin','adsr-original-instruction-probes.json','adsr-expanded-original-probes.json','adsr-user-prepare-probes-crt.json','adsr-reset-probes.json']
def read(name):return json.loads((E/name).read_text(encoding='utf-8'))
def flat(state):
 state=state.copy()
 for prefix in ['const_d','const_r']:
  if prefix in state:
   for i,v in enumerate(state.pop(prefix)):state[prefix+str(i)]=v
 return state
def values(state):
 state=flat(state);return [state.get(k,0) for k in FIELDS]
cmd=[]
def add(op,args=b'',expected=None):
 expected=flat(expected or {});mask=sum(1<<i for i,k in enumerate(FIELDS) if k in expected)
 cmd.append(struct.pack('<B',op)+args+struct.pack('<Q',mask)+struct.pack('<33q',*values(expected)))
def construct():add(0)
def prepare(words,mask=511,expected=None):add(1,struct.pack('<10H',*words,mask),expected)
def load(state):add(6,struct.pack('<33q',*values(state)))
ctor=(E/'adsr-constructor-emulated-crt.bin').read_bytes()
def i16(v):return (v+32768)%65536-32768
def preset(variant,time,sustain):
 state={k:0 for k in FIELDS};curve=ctor[(0xa0,0x7b0)[variant]:][:0x710];idx=(i16(time)>>8)&127;frac=time&255;a,b=struct.unpack_from('<2h',curve,0x60c+idx*2);aux=i16(a+((b-a)*frac>>8));const=[(786,-4521,155),(5046,-3276,-28)][variant]
 state.update(time_a=i16(time),time_d=i16(time),time_r=i16(time),sustain=i16(sustain),variant_a=variant,variant_d=variant,variant_r=variant,aux_d=aux,aux_r=aux,dynamic_d=32767,dynamic_r=32767)
 for prefix in ['const_d','const_r']:
  for i,v in enumerate(const):state[prefix+str(i)]=v
 return state
old=read('adsr-original-instruction-probes.json')
for case in old['gate_cases']:
 state=preset(0,8192,16384);state.update({k:v for k,v in case['input'].items() if k in FIELDS});load(state);add(2,bytes([case['input']['gate']]),case['output'])
for case in old['sequences']:
 load(preset(case['curve_variant'],case['prepared_time'],case['sustain_q15']));events={int(k):v for k,v in case['gate_events'].items()}
 for rec in case['trace']:
  if rec['tick'] in events:add(2,bytes([events[rec['tick']]]))
  add(3,expected={k:v for k,v in rec.items() if k!='tick'})
expanded=read('adsr-expanded-original-probes.json');baseline=expanded['dirty_mask_cases'][0]['state']
for case in expanded['dirty_mask_cases']:
 construct();prepare(expanded['baseline_words'],expected=baseline);prepare(expanded['replacement_words'],case['mask'],case['state'])
for case in expanded['scenarios']:
 construct();prepare(case['initial_words']);events={int(k):v for k,v in case['events'].items()};holds={int(k):v for k,v in case.get('holds',{}).items()};controls={int(k):v for k,v in case.get('controls',{}).items()}
 for rec in case['trace']:
  tick=rec['tick']
  if tick in holds:add(4,bytes([holds[tick]]))
  if tick in controls:prepare(controls[tick]['words'],controls[tick]['mask'])
  if tick in events:add(2,bytes([events[tick]]))
  add(3,expected=rec['state'])
for case in read('adsr-user-prepare-probes-crt.json')['cases']:
 construct();expected={}
 names={'prepared_attack':'time_a','prepared_decay':'time_d','prepared_sustain':'sustain','prepared_release':'time_r','decay_aux':'aux_d','release_aux':'aux_r','decay_constants':'const_d','release_constants':'const_r'}
 for k,v in names.items():expected[v]=case[k]
 prepare([case['word']]*4+[case['curve_selector_word']]*3+[0,0],expected=expected)
# Original reset only observes low0xa0 bytes; typed fields are decoded from those
# explicit raw patterns. No invented native constructor default is asserted.
offsets=[(0x34,'i'),(0x38,'i'),(0x40,'i'),(0x3e,'h'),(0x30,'h'),(0x90,'B'),(0x91,'B'),(0x92,'B'),(0x93,'B'),(0x87,'B'),(0x7e,'h'),(0x80,'h'),(0x82,'h'),(0x84,'h'),(0x94,'i'),(0x98,'i'),(0x9c,'i'),(0x50,'h'),(0x66,'h'),(0x60,'h'),(0x62,'h'),(0x64,'h'),(0x78,'h'),(0x7a,'h'),(0x7c,'h'),(0x88,'h'),(0x8a,'h'),(0x44,'i'),(0x48,'q'),(0x54,'i'),(0x58,'q'),(0x68,'i'),(0x70,'q')]
for case in read('adsr-reset-probes.json')['cases']:
 raw=bytes([case['fill_byte']])*0xa0;state={k:struct.unpack_from('<'+t,raw,o)[0] for k,(o,t) in zip(FIELDS,offsets)};load(state);state.update(phase=0,value_q31=0,current_q15=0,hold=0,pending_release=0);add(5,expected=state)
blob=b'ADSRFX01'+ctor[0xa8:0x7b0]+ctor[0x7b8:0xec0]+struct.pack('<I',len(cmd))+b''.join(cmd);(ROOT/'original-fixtures.bin').write_bytes(blob)
manifest={'schema':'ADSRFX01','commands':len(cmd),'expected_fields':FIELDS,'original_sources':[{ 'path':n,'sha256':hashlib.sha256((E/n).read_bytes()).hexdigest()} for n in SOURCES],'fixture_sha256':hashlib.sha256(blob).hexdigest(),'fixture_bytes':len(blob),'semantics':'Expected values serialized only from retained original instruction/SystemCRT fixtures. Legacy prepared-state initialization decodes auxiliary from original constructor bytes; test-only fixture init is explicit. No candidate/scalar module imported.'};(ROOT/'fixture-manifest.json').write_text(json.dumps(manifest,indent=2));print('Serialized',len(cmd),'commands',len(blob),'bytes')
# Generate only field adapters, not any behavioral expectations.
flatten='\n'.join('  static_cast<std::int64_t>(s.'+k+'),' for k in CPP)
assign='\n'.join('  s.'+k+'=static_cast<decltype(s.'+k+')>(v['+str(i)+']);' if '[' not in k else '  s.'+k+'=static_cast<std::int16_t>(v['+str(i)+']);' for i,k in enumerate(CPP))
(ROOT/'field-adapters.inc').write_text('std::array<std::int64_t,33> flatten(const AdsrState& s) { return {\n'+flatten+'\n};}\nAdsrState state_from(const std::array<std::int64_t,33>& v) { AdsrState s;\n'+assign+'\nreturn s;}\n')
