"""Acquire pinned official Arturia GUI assets without running any installer.
Python standard library only. Downloads public HTTPS byte ranges, verifies XAR
TOC/payload SHA256, reads regular odc-CPIO members, applies only pinned text
newline conversions, and verifies every final file. No auth/account/app required.
"""
from pathlib import Path, PurePosixPath
import argparse, gzip, hashlib, json, struct, time, urllib.request, zlib

def sha(path):
 h=hashlib.sha256()
 with path.open('rb') as f:
  while block:=f.read(1024*1024):h.update(block)
 return h.hexdigest()

def response(url,start,length,total):
 end=start+length-1
 r=urllib.request.urlopen(urllib.request.Request(url,headers={'Range':f'bytes={start}-{end}','User-Agent':'MiniFreak-Asset-Verification/1.0'}),timeout=60)
 if r.status!=206 or r.headers.get('Content-Range')!=f'bytes {start}-{end}/{total}':
  r.close();raise ValueError('Server did not return the pinned byte range')
 return r

def exact(stream,count):
 b=stream.read(count)
 if len(b)!=count:raise ValueError('Truncated package stream')
 return b

def main():
 parser=argparse.ArgumentParser();parser.add_argument('--manifest',type=Path,default=Path(__file__).with_name('acquisition-manifest.json'));parser.add_argument('--output',type=Path,required=True);parser.add_argument('--cache',type=Path,required=True);args=parser.parse_args()
 manifest=json.loads(args.manifest.read_text(encoding='utf-8-sig'));args.output.mkdir(parents=True,exist_ok=True);args.cache.mkdir(parents=True,exist_ok=True)
 root=args.output.resolve();url=manifest['package_url'];total=manifest['package_total_bytes'];heap=manifest['package_xar_heap_offset']
 with response(url,0,heap,total) as r:prefix=exact(r,heap)
 magic,size,version,compressed,uncompressed,algorithm=struct.unpack('>4sHHQQI',prefix[:28])
 if magic!=b'xar!' or size!=28 or version!=1 or size+compressed!=heap:raise ValueError('Unexpected XAR header')
 toc=zlib.decompress(prefix[size:]);assert len(toc)==uncompressed
 if hashlib.sha256(toc).hexdigest()!=manifest['decompressed_toc_sha256']:raise ValueError('Official package TOC identity changed')
 wanted={r['path']:r for r in manifest['resources']};found={};payload_proofs=[]
 for p in manifest['payloads']:
  file=args.cache/(p['package'].replace(' ','_')+'.payload.cpio.gz')
  if not file.exists() or sha(file)!=p['sha256']:
   part=file.with_suffix(file.suffix+'.part')
   for attempt in range(2):
    try:
     with response(url,p['range_start'],p['range_length'],total) as source,part.open('wb') as dest:
      while block:=source.read(1024*1024):dest.write(block)
     if part.stat().st_size!=p['range_length'] or sha(part)!=p['sha256']:raise ValueError('Payload hash/size mismatch')
     part.replace(file);break
    except OSError:
     if attempt==0:time.sleep(5);continue
     raise
  payload_proofs.append({'package':p['package'],'sha256':sha(file),'bytes':file.stat().st_size})
  with gzip.open(file,'rb') as source:
   while True:
    header=exact(source,76)
    if header[:6]!=b'070707':raise ValueError('Expected odc CPIO')
    mode=int(header[18:24],8);namesize=int(header[59:65],8);length=int(header[65:76],8)
    if not 1<=namesize<=1048576:raise ValueError('Invalid CPIO path length')
    name=exact(source,namesize).rstrip(b'\0').decode('utf-8')
    if name=='TRAILER!!!':break
    prefix=manifest['member_prefix'];relative=name[len(prefix):] if name.startswith(prefix) else None
    keep=relative in wanted and (mode&0o170000)==0o100000
    if keep:
     spec=wanted[relative]
     if spec['payload']!=p['package'] or relative in found:raise ValueError('Unexpected duplicate/member payload')
     data=exact(source,length)
     if hashlib.sha256(data).hexdigest()!=spec['package_sha256']:raise ValueError('Source member hash mismatch: '+relative)
     if spec['transform']=='lf_to_crlf':data=data.replace(b'\r\n',b'\n').replace(b'\n',b'\r\n')
     elif spec['transform']!='identity':raise ValueError('Unknown transform')
     if hashlib.sha256(data).hexdigest()!=spec['sha256']:raise ValueError('Final installed-resource hash mismatch: '+relative)
     safe=PurePosixPath(relative)
     if safe.is_absolute() or '..' in safe.parts:raise ValueError('Unsafe output path')
     target=root.joinpath(*safe.parts)
     if not target.resolve().is_relative_to(root):raise ValueError('Output escapes destination')
     target.parent.mkdir(parents=True,exist_ok=True)
     for attempt in range(2):
      try:target.write_bytes(data);break
      except OSError:
       if attempt==0:time.sleep(5);continue
       raise
     found[relative]=spec['sha256']
    else:
     while length:
      block=exact(source,min(length,1024*1024));length-=len(block)
 if set(found)!=set(wanted):raise ValueError('Missing required resources: '+str(sorted(set(wanted)-set(found))))
 report={'verified_runtime_resources':len(found),'verified_font_files':sum(p.startswith('fonts/') for p in found),'excluded_nonruntime':manifest['excluded_nonruntime'],'package_url':url,'payloads':payload_proofs,'final_resource_hashes':found,'installer_executed':False,'authentication_used':False}
 (args.cache/'acquisition-result.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps({k:v for k,v in report.items() if k not in ['final_resource_hashes','payloads']},indent=2))
if __name__=='__main__':main()
