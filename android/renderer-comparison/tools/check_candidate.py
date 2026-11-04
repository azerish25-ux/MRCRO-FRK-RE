"""Asset-free static checks. This does NOT compile/run Android or prove raster parity."""
from pathlib import Path
import ast,hashlib,json,re,xml.etree.ElementTree as E
P=Path(__file__).resolve().parent.parent
for p in (P/'tools').glob('*.py'):ast.parse(p.read_text(encoding='utf-8-sig'),filename=str(p))
for p in (P/'app/src').rglob('*.xml'):E.parse(p)
s=json.loads((P/'app/src/main/assets/filter-slice.json').read_text());m=json.loads((P/'tools/acquisition-manifest.json').read_text());subset=json.loads((P/'tools/renderer-assets.json').read_text());allhashes={x['path']:x['sha256'] for x in m['resources']}
assert len(s['knobs'])==3
assert [k['parameter'] for k in s['knobs']]==['Vcf_Cutoff','Vcf_Resonance','Vcf_EnvAmount']
assert [k['factory_normalized'] for k in s['knobs']]==[1.0,0.0,0.5]
for k in s['knobs']:
 assert k['bounds'][2:]==[125,125]
 assert k['style']['ring-gradient-color']=='cecece'
 assert k['style']['center-outline-gradient-start-color']=='d5d5d5'
 assert k['style']['center-outline-gradient-end-color']=='525252'
 assert k['style']['angle']=='2.4'
 assert k['bounds'][0]>=s['crop'][0] and k['bounds'][0]+125<=s['crop'][0]+s['crop'][2]
 assert k['label']['bounds'][1]+k['label']['bounds'][3]<=s['crop'][1]+s['crop'][3]
for path,sha in subset.items():assert allhashes[path]==sha
java=list((P/'app/src').rglob('*.java'))
for p in java:
 text=p.read_text(encoding='utf-8-sig');assert 'package com.mfrk.renderercomparison;' in text
 assert 'readAllBytes(' not in text and 'Typeface.DEFAULT' not in text
 # Basic delimiter scan after stripping comments/string/character literals.
 tokens=re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'','',text)
 stack=[];pair={'}':'{',')':'(',']':'['}
 for ch in tokens:
  if ch in '{([':stack.append(ch)
  elif ch in '})]':assert stack and stack.pop()==pair[ch],str(p)
 assert not stack,str(p)
assert not list((P/'app/src').rglob('*.ttf')) and not list((P/'app/src').rglob('*.otf')) and not list((P/'app/src').rglob('*.png'))
assert 'LAUNCHER' not in (P/'app/src/main/AndroidManifest.xml').read_text()
assert "version '8.7.3'" in (P/'build.gradle').read_text()
report={'static_checks':'passed','java_files':len(java),'source_controls':3,'selected_vendor_assets':len(subset),'vendor_binary_assets_in_source':0,'android_compile_run':'not attempted locally; GitHub Actions required','scope':'source consistency/XML/Python parsing/Java delimiters only, not Java or Android compilation'}
(P/'provenance/static-checks.json').write_text(json.dumps(report,indent=2),encoding='utf-8',newline='\n');print(json.dumps(report))
