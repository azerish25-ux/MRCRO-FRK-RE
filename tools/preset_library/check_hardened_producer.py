"""Focused R1/R2 tests. Filesystem effects stay under the explicitly supplied owned work root."""
from pathlib import Path
import copy
import argparse
import gzip
import hashlib
import importlib.util
import io
import json
import os
import struct
import subprocess
import sys
import uuid

BASE = Path(__file__).resolve().parent
TOOL = BASE/'acquire_library.py'
SPEC = importlib.util.spec_from_file_location('hardened_producer', TOOL)
MOD = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MOD
SPEC.loader.exec_module(MOD)
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--work-root',type=Path,required=True)
parser.add_argument('--report',type=Path,required=True)
args=parser.parse_args()
args.work_root.mkdir(parents=True,exist_ok=True)
args.report.parent.mkdir(parents=True,exist_ok=True)
WORK = args.work_root.resolve()/('focused-'+uuid.uuid4().hex)
WORK.mkdir()
RESULTS = []
MANIFEST = json.loads((BASE/'acquisition-manifest.json').read_text())

class NetworkReached(Exception):
    pass

def no_network(*args):
    raise NetworkReached()

def invoke(module, manifest, folder):
    folder.mkdir(parents=True, exist_ok=True)
    path = folder/'manifest.json'
    path.write_text(json.dumps(manifest), encoding='utf-8')
    sys.argv = ['acquire_library.py','--manifest',str(path),'--output',str(folder/'out'),'--cache',str(folder/'cache')]
    module.main()

def reject_manifest(name, mutate):
    altered = copy.deepcopy(MANIFEST)
    mutate(altered)
    folder = WORK/name
    MOD.response = no_network
    try:
        invoke(MOD, altered, folder)
    except NetworkReached:
        raise AssertionError(name+' reached network')
    except (ValueError, UnicodeError, TypeError) as error:
        assert not (folder/'out').exists() and not (folder/'cache').exists()
        RESULTS.append({'case':name,'result':'rejected_before_network_or_write','error':str(error)})
    else:
        raise AssertionError(name+' accepted')

def set_path(m, path):
    m['files'][0]['output_path'] = path
    m['files'][0]['path'] = path.split('/',1)[1] if '/' in path else path

def casefold(m):
    value = m['files'][0]['path'].upper()
    m['files'][1].update(path=value, output_path='bank_images/'+value)

def unicode_alias(m):
    m['files'][0].update(path='Caf\u00e9.png',output_path='bank_images/Caf\u00e9.png')
    m['files'][1].update(path='Cafe\u0301.png',output_path='bank_images/Cafe\u0301.png')

def directory_case(m):
    m['files'][0].update(path='Dir/a.png',output_path='bank_images/Dir/a.png')
    m['files'][1].update(path='dir/b.png',output_path='bank_images/dir/b.png')

def prefix_conflict(m):
    m['files'][0].update(path='node',output_path='bank_images/node')
    m['files'][1].update(path='node/child.png',output_path='bank_images/node/child.png')

reject_manifest('true_casefold_collision',casefold)
reject_manifest('unicode_canonical_collision',unicode_alias)
reject_manifest('directory_case_alias',directory_case)
reject_manifest('file_directory_prefix',prefix_conflict)
for name, path in {
    'empty_filename':'bank_images/', 'empty_component':'bank_images//a.png',
    'dot_component':'bank_images/./a.png', 'parent_component':'bank_images/../a.png',
    'trailing_space':'bank_images/a.png ', 'trailing_dot':'bank_images/a.png.',
    'ads':'bank_images/a.png:stream', 'device_con':'bank_images/CON.png',
    'device_com':'bank_images/COM1.bin', 'device_superscript':'bank_images/LPT\u00b2.png',
    'device_suffix_space':'bank_images/NUL .png', 'drive':'C:/escape.png',
    'unc':'//server/share/a.png','backslash':'bank_images/a\\b.png',
    'too_long':'bank_images/'+('x'*256), 'control':'bank_images/a\x01.png',
}.items():
    reject_manifest(name,lambda m,value=path:set_path(m,value))

def package_label(m, value, index=0):
    previous = m['payloads'][index]['package']
    m['payloads'][index]['package'] = value
    for row in m['files']:
        if row['payload'] == previous:
            row['payload'] = value

for name,value in {
    'cache_traversal':'../evil.pkg','cache_drive':'C:evil.pkg',
    'cache_separator':'sub/evil.pkg','cache_backslash':'sub\\evil.pkg',
    'cache_reserved':'CON.pkg','cache_trailing_dot':'evil.pkg.',
}.items():
    reject_manifest(name,lambda m,value=value:package_label(m,value))
def cache_collision(m):
    package_label(m,'A B.pkg',0)
    package_label(m,'A_B.pkg',1)
reject_manifest('cache_key_conversion_collision',cache_collision)
def cache_case(m):
    package_label(m,'Same.pkg',0)
    package_label(m,'same.pkg',1)
reject_manifest('cache_key_case_collision',cache_case)

# Small pinned streams exercise the real download/member/report paths without
# repeating large official downloads or modifying any reference identity.
def cpio_member(name, data, mode=0o100644):
    encoded = name.encode()+b'\0'
    header = b'070707'+(''.join(f'{v:06o}' for v in [0,1,mode,0,0,1,0])+f'{0:011o}{len(encoded):06o}{len(data):011o}').encode()
    assert len(header)==76
    return header+encoded+data

def toy():
    data = b'owned exact bytes\x00with interior\nLF\n'
    member = './tmp/officially-shaped/member'
    payload = gzip.compress(cpio_member(member,data)+cpio_member('TRAILER!!!',b'',0))
    toc = b'<xar><toc/></xar>'
    compressed = __import__('zlib').compress(toc)
    prefix = struct.pack('>4sHHQQI',b'xar!',28,1,len(compressed),len(toc),1)+compressed
    sha = hashlib.sha256(data).hexdigest()
    manifest = {'schema':'arturia.minifreak-v.complete-preset-library.v1','package_url':'https://example.invalid/pinned.pkg','package_total_bytes':len(prefix)+len(payload),'package_xar_heap_offset':len(prefix),'decompressed_toc_sha256':hashlib.sha256(toc).hexdigest(),'total_runtime_files':1,'expected_role_counts':{'presets':1},'preset_archive_count':1,'browser_catalog_preset_count':1,'tutorial_state_count':0,'hardware_backup_preset_count':0,'runtime_bytes':len(data),'payloads':[{'package':'Safe.pkg','range_start':len(prefix),'range_length':len(payload),'sha256':hashlib.sha256(payload).hexdigest()}],'files':[{'role':'presets','path':'Factory/Exact','output_path':'presets/Factory/Exact','payload':'Safe.pkg','member_path':member,'package_bytes':len(data),'package_sha256':sha,'bytes':len(data),'sha256':sha,'transform':'identity'}]}
    def streams(url,start,length,total):
        assert url==manifest['package_url'] and total==manifest['package_total_bytes']
        if start==0:
            assert length==len(prefix)
            return io.BytesIO(prefix)
        assert start==len(prefix) and length==len(payload)
        return io.BytesIO(payload)
    return manifest, streams, data

for kind in ['output_part_hardlink','cache_part_hardlink','report_hardlink','output_target_hardlink','cache_target_hardlink']:
    folder = WORK/kind
    folder.mkdir()
    stage, cache = folder/'out', folder/'cache'
    stage.mkdir();cache.mkdir()
    sentinel = folder/'outside-intended-root-sentinel'
    sentinel.write_bytes(b'untouched sentinel')
    if kind=='output_part_hardlink':
        target=stage/'presets/Factory/Exact.part'
        target.parent.mkdir(parents=True)
    elif kind=='cache_part_hardlink':
        target=cache/'Safe.pkg.payload.cpio.gz.part'
    elif kind=='report_hardlink':
        target=cache/MOD.REPORT_NAME
    elif kind=='output_target_hardlink':
        target=stage/'presets/Factory/Exact'
        target.parent.mkdir(parents=True)
    else:
        target=cache/'Safe.pkg.payload.cpio.gz'
    os.link(sentinel,target)
    manifest, streams, expected = toy()
    MOD.response = streams
    try:
        invoke(MOD,manifest,folder)
        result='safe_success'
    except ValueError as error:
        result='safe_rejection: '+str(error)
    assert sentinel.read_bytes()==b'untouched sentinel',kind
    assert target.exists(),kind+' deleted unrelated path'
    if kind=='cache_part_hardlink':
        assert (stage/'presets/Factory/Exact').read_bytes()==expected
    RESULTS.append({'case':kind,'result':result,'sentinel_unchanged':True,'preexisting_link_preserved':True})

# Symbolic-link/junction cases use ONLY links and targets owned by this test.
def link_directory(target, link):
    try:
        os.symlink(target,link,target_is_directory=True)
        return 'symlink'
    except OSError as error:
        if os.name!='nt':
            raise
        # Same native PowerShell creation end-to-end, no destructive operation.
        source=str(target).replace("'","''");dest=str(link).replace("'","''")
        command=f"New-Item -ItemType Junction -Path '{dest}' -Target '{source}' | Out-Null"
        result=subprocess.run(['pwsh','-NoProfile','-Command',command],capture_output=True,text=True)
        if result.returncode:
            raise RuntimeError('Cannot create owned junction: '+result.stderr) from error
        return 'junction'

for kind in ['output_parent_reparse','cache_root_reparse','output_root_reparse']:
    folder=WORK/kind;folder.mkdir();outside=folder/'owned-outside';outside.mkdir();sentinel=outside/'sentinel';sentinel.write_bytes(b'unchanged')
    stage,cache=folder/'out',folder/'cache'
    if kind=='output_parent_reparse':
        stage.mkdir();form=link_directory(outside,stage/'presets')
    elif kind=='cache_root_reparse':
        form=link_directory(outside,cache)
    else:
        form=link_directory(outside,stage)
    manifest,_,_=toy();MOD.response=no_network
    try:invoke(MOD,manifest,folder)
    except NetworkReached:raise AssertionError(kind+' reached network')
    except ValueError:pass
    else:raise AssertionError(kind+' accepted')
    assert sentinel.read_bytes()==b'unchanged' and set(p.name for p in outside.iterdir())=={'sentinel'}
    RESULTS.append({'case':kind,'link_form':form,'result':'rejected_before_network','sentinel_unchanged':True})

# Direct exact writer exercises exclusive random temp cleanup and retry behavior.
for kind in ['interrupted_then_retry','failed_twice','uncertain_committed_replace']:
    folder=WORK/kind;folder.mkdir();tree=MOD.SafeTree(folder/'stage');data=b'verified final data';sha=hashlib.sha256(data).hexdigest();calls=[];delays=[]
    previous_sleep=MOD.time.sleep;previous_replace=MOD.os.replace
    MOD.time.sleep=lambda delay:delays.append(delay)
    def writer(stream):
        calls.append(1)
        if kind=='failed_twice' or kind=='interrupted_then_retry' and len(calls)==1:
            stream.write(b'partial')
            raise OSError('Owned simulated storage interruption')
        stream.write(data)
    if kind=='uncertain_committed_replace':
        def committed_then_error(source,target):
            previous_replace(source,target)
            raise OSError('Owned uncertain successful rename')
        MOD.os.replace=committed_then_error
    try:
        try:
            MOD.atomic_write(tree,('result.bin',),writer,len(data),sha)
            assert kind!='failed_twice'
            assert (tree.root/'result.bin').read_bytes()==data
        except OSError:
            assert kind=='failed_twice'
            assert not (tree.root/'result.bin').exists()
        assert not list(tree.root.glob('.mfrk-acquire-*.tmp'))
        assert delays==[5]
        assert len(calls)==(1 if kind=='uncertain_committed_replace' else 2)
    finally:
        MOD.time.sleep=previous_sleep;MOD.os.replace=previous_replace
    RESULTS.append({'case':kind,'result':'correct_retry_or_safe_failure','writer_calls':len(calls),'delays':delays,'owned_temps_remaining':0})

# A replaced temp is not ours to unlink; cleanup must preserve it.
folder=WORK/'cleanup_identity';folder.mkdir();path=folder/'temp';path.write_bytes(b'ours');owned=MOD.identity(path.stat());os.replace(path,folder/'moved-owned-temp');path.write_bytes(b'unrelated replacement');MOD.cleanup_owned_temp(path,owned);assert path.read_bytes()==b'unrelated replacement'
RESULTS.append({'case':'cleanup_replaced_temp_identity','result':'unrelated replacement preserved'})

# Explicit file symlinks complement the retained hardlink cases.
for kind in ['direct_output_part_symlink','cache_part_symlink','report_symlink','cache_target_symlink']:
    folder=WORK/kind;folder.mkdir();stage=folder/'out';cache=folder/'cache';stage.mkdir();cache.mkdir()
    sentinel=folder/'outside-root-sentinel';sentinel.write_bytes(b'untouched')
    if kind=='direct_output_part_symlink':
        target=stage/'asset.bin.part'
    elif kind=='cache_part_symlink':
        target=cache/'Safe.pkg.payload.cpio.gz.part'
    elif kind=='report_symlink':
        target=cache/MOD.REPORT_NAME
    else:
        target=cache/'Safe.pkg.payload.cpio.gz'
    try:
        os.symlink(sentinel,target)
    except OSError as error:
        if getattr(error,'winerror',None)!=1314:
            raise
        RESULTS.append({'case':kind,'result':'platform_unavailable','winerror':1314,'reason':'Current Windows token cannot create file symlinks; no privilege change attempted.'})
        continue
    manifest,streams,expected=toy();MOD.response=streams
    try:
        if kind=='direct_output_part_symlink':
            MOD.save_exact(stage/'asset.bin',expected,hashlib.sha256(expected).hexdigest())
        else:
            invoke(MOD,manifest,folder)
        result='safe_success'
    except ValueError as error:
        result='safe_rejection: '+str(error)
    assert sentinel.read_bytes()==b'untouched' and target.is_symlink()
    RESULTS.append({'case':kind,'result':result,'sentinel_unchanged':True,'preexisting_symlink_preserved':True})

if os.name=='nt':
    folder=WORK/'explicit_windows_junction';folder.mkdir();stage=folder/'out';stage.mkdir();outside=folder/'owned-outside';outside.mkdir();sentinel=outside/'sentinel';sentinel.write_bytes(b'untouched');link=stage/'presets'
    assert link.resolve(strict=False).is_relative_to(WORK) and outside.resolve().is_relative_to(WORK)
    dest=str(link).replace("'","''");source=str(outside).replace("'","''")
    command=f"New-Item -ItemType Junction -Path '{dest}' -Target '{source}' | Out-Null"
    process=subprocess.run(['pwsh','-NoProfile','-Command',command],capture_output=True,text=True)
    assert process.returncode==0,process.stderr
    manifest,_,_=toy();MOD.response=no_network
    try:invoke(MOD,manifest,folder)
    except NetworkReached:raise AssertionError('junction admitted')
    except ValueError:pass
    else:raise AssertionError('junction accepted')
    assert sentinel.read_bytes()==b'untouched' and set(p.name for p in outside.iterdir())=={'sentinel'}
    RESULTS.append({'case':'explicit_windows_junction','result':'rejected_before_network','sentinel_unchanged':True})

# File reparse policy is still directly exercised without claiming a real
# file-symlink creation that this non-elevated Windows token cannot perform.
import types
for mode,attrs in [(0o120777,0),(0o100644,0x400)]:
    fake=types.SimpleNamespace(st_mode=mode,st_file_attributes=attrs,st_nlink=1)
    try:MOD.check_info(WORK/'owned-policy-check',fake,False)
    except ValueError:pass
    else:raise AssertionError('file link/reparse policy accepted')
RESULTS.append({'case':'file_symlink_and_reparse_policy','result':'both metadata forms rejected','actual_link_creation_claimed':False})

# A preexisting candidate-name collision is never reused or cleaned up.
folder=WORK/'exclusive_temp_collision';folder.mkdir();tree=MOD.SafeTree(folder/'stage');tree.root.mkdir();sentinel=folder/'outside-root-sentinel';sentinel.write_bytes(b'untouched');collision=tree.root/'.mfrk-acquire-collision.tmp';os.link(sentinel,collision)
old_names=MOD.tempfile._get_candidate_names
MOD.tempfile._get_candidate_names=lambda:iter(['collision','fresh-owned'])
try:
    payload=b'new exact bytes';MOD.atomic_write(tree,('asset.bin',),lambda stream:stream.write(payload),len(payload),hashlib.sha256(payload).hexdigest())
finally:MOD.tempfile._get_candidate_names=old_names
assert sentinel.read_bytes()==b'untouched' and collision.exists() and not(tree.root/'.mfrk-acquire-fresh-owned.tmp').exists()
RESULTS.append({'case':'exclusive_random_name_collision','result':'preexisting collision untouched; fresh temp used'})

folder=WORK/'bad_completed_hash';folder.mkdir();tree=MOD.SafeTree(folder/'stage')
try:MOD.atomic_write(tree,('asset.bin',),lambda stream:stream.write(b'wrong'),5,'0'*64)
except ValueError:pass
else:raise AssertionError('bad completed hash accepted')
assert not(tree.root/'asset.bin').exists() and not list(tree.root.glob('.mfrk-acquire-*.tmp'))
RESULTS.append({'case':'bad_completed_hash','result':'no accepted partial and owned temp removed'})

report={'tool_sha256':hashlib.sha256(TOOL.read_bytes()).hexdigest(),'workspace':str(WORK),'cases':RESULTS,'case_count':len(RESULTS),'test_links_and_sentinels_confined_to_work_root':True,'explicit_report_path':str(args.report)}
args.report.write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps({'case_count':len(RESULTS),'tool_sha256':report['tool_sha256'],'workspace':str(WORK),'all_available_tests_passed':True,'platform_unavailable_cases':sum(r.get('result')=='platform_unavailable' for r in RESULTS)},indent=2))
