"""Losslessly convert canonical ADSRFX01 to changed-field/varint ADSRFX02."""
import hashlib,json,pathlib,struct,argparse
ROOT=pathlib.Path(__file__).resolve().parent
HEADER_BYTES=8+3600+4
ARGUMENT_BYTES={0:0,1:20,2:1,3:0,4:1,5:0,6:None}
class Reader:
 def __init__(self,data):self.data=data;self.pos=0
 def take(self,n):
  value=self.data[self.pos:self.pos+n]
  if len(value)!=n:raise ValueError('truncated fixture')
  self.pos+=n;return value
 def varint(self):
  value=0
  for index in range(10):
   byte=self.take(1)[0];payload=byte&127
   if index==9 and payload>1:raise ValueError('varint exceeds uint64')
   value|=payload<<(index*7)
   if byte<128:return value
  raise ValueError('varint exceeds ten bytes')
def encode_varint(value):
 if not 0<=value<(1<<64):raise ValueError('not uint64')
 result=bytearray()
 while value>=128:result.append((value&127)|128);value>>=7
 result.append(value);return bytes(result)
def zigzag(value):return ((value<<1)^(value>>63))&((1<<64)-1)
def unzigzag(value):return (value>>1)^-(value&1)
def transform(data,compress):
 reader=Reader(data);header=reader.take(HEADER_BYTES);expected=b'ADSRFX01' if compress else b'ADSRFX02'
 if header[:8]!=expected:raise ValueError('fixture magic')
 out=bytearray((b'ADSRFX02' if compress else b'ADSRFX01')+header[8:]);commands=struct.unpack_from('<I',header,HEADER_BYTES-4)[0]
 previous=[0]*33;calls=0;changed=0
 def fields():
  nonlocal calls,changed
  calls+=1
  if compress:
   current=list(struct.unpack('<33q',reader.take(264)));bitmap=sum(1<<i for i in range(33) if current[i]!=previous[i]);out.extend(encode_varint(bitmap))
   for i,value in enumerate(current):
    if bitmap&(1<<i):out.extend(encode_varint(zigzag(value)));changed+=1
   previous[:]=current
  else:
   bitmap=reader.varint()
   if bitmap>>33:raise ValueError('field bitmap exceeds 33 bits')
   for i in range(33):
    if bitmap&(1<<i):previous[i]=unzigzag(reader.varint());changed+=1
   out.extend(struct.pack('<33q',*previous))
 for _ in range(commands):
  opcode=reader.take(1);out.extend(opcode);op=opcode[0]
  if op not in ARGUMENT_BYTES:raise ValueError('invalid opcode')
  if op==6:fields()
  else:out.extend(reader.take(ARGUMENT_BYTES[op]))
  out.extend(reader.take(8));fields()
 if reader.pos!=len(data):raise ValueError('trailing fixture data')
 return bytes(out),{'commands':commands,'fields_calls':calls,'changed_cells':changed}
def verify_codec_boundaries():
 values=[-(1<<63),-4294967296,-1,0,1,4294967296,(1<<63)-1]
 for value in values:
  encoded=encode_varint(zigzag(value));reader=Reader(encoded);assert unzigzag(reader.varint())==value;assert reader.pos==len(encoded)
 rejected=[]
 for name,data in [('ten_continuations',bytes([128])*10),('uint64_overflow',bytes([255])*9+b'\x02'),('truncated_varint',b'\x80')]:
  try:Reader(data).varint()
  except ValueError as error:rejected.append({'case':name,'reason':str(error)})
  else:raise AssertionError(name+' accepted')
 return {'signed_boundary_values':values,'malformed_varints_rejected':rejected}
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--input',type=pathlib.Path,required=True);parser.add_argument('--output',type=pathlib.Path,required=True);parser.add_argument('--report',type=pathlib.Path,required=True);args=parser.parse_args()
 codec=verify_codec_boundaries();original=args.input.read_bytes();compact,stats=transform(original,True);restored,reverse_stats=transform(compact,False)
 if original!=restored:raise AssertionError('FX02 roundtrip changed canonical bytes')
 assert stats==reverse_stats
 # Publish only after full-byte equality, not merely behavior/mask equality.
 args.output.write_bytes(compact)
 report={'canonical_bytes':len(original),'compact_bytes':len(compact),'bytes_saved':len(original)-len(compact),'compact_fraction':len(compact)/len(original),'canonical_sha256':hashlib.sha256(original).hexdigest(),'compact_sha256':hashlib.sha256(compact).hexdigest(),'restored_sha256':hashlib.sha256(restored).hexdigest(),'byte_identical_roundtrip':True,'preserved_all_33_cells_per_fields_call':True,**stats,'codec_checks':codec,'format':'ADSRFX02; header/table bytes/command count/opcode arguments/masks unchanged except magic01->02. Every fields() array shares one global previous initialized to33 zeros; changed bitmap unsignedvarint then ZigZag uint64varints in ascending cell order. Bounds: ten bytes per varint, bitmap33 bits. No compression library.'}
 args.report.write_text(json.dumps(report,indent=2),encoding='utf-8')
 print(json.dumps({k:report[k] for k in ['canonical_bytes','compact_bytes','compact_sha256','byte_identical_roundtrip','commands','fields_calls']},indent=2))
if __name__=='__main__':main()
