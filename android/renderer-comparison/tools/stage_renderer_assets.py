"""Stage only three hash-verified renderer inputs into generated build assets."""
from pathlib import Path,PurePosixPath
import argparse,hashlib,json
p=argparse.ArgumentParser();p.add_argument('--verified-root',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
manifest=json.loads(Path(__file__).with_name('renderer-assets.json').read_text());a.output.mkdir(parents=True,exist_ok=True)
for relative,wanted in manifest.items():
 path=PurePosixPath(relative);assert not path.is_absolute() and '..' not in path.parts
 data=a.verified_root.joinpath(*path.parts).read_bytes();actual=hashlib.sha256(data).hexdigest()
 if actual!=wanted:raise ValueError('Renderer input mismatch: '+relative)
 dest=a.output/'vendor'/Path(*path.parts);assert dest.resolve().is_relative_to(a.output.resolve());dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(data)
print('Staged',len(manifest),'verified binary assets outside source directories')
