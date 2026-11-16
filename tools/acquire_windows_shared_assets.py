"""Acquire exact shared GUI bytes from the pinned official Windows package.

The installer is never executed. Only its Shared/images member is extracted;
vendor files remain outside source git. Windows output/cache paths must be on D:.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import urllib.request

PACKAGE_URL = "https://dl.arturia.net/products/minifreak-v/soft/MiniFreak_V__5_1_1_6945.exe"
PACKAGE_BYTES = 513504824
PACKAGE_SHA256 = "989e89c941bf5148ef3a0ca72f08a327ea0cee3bbe40ed4abcfc0702a9878c5e"
MEMBER_PARTS = ("commonappdata", "Arturia", "Shared", "images")


def digest(path: Path) -> str:
    result = hashlib.sha256()
    with path.open("rb") as stream:
        while block := stream.read(1024 * 1024):
            result.update(block)
    return result.hexdigest()


def output_path(path: Path) -> Path:
    path = path.resolve()
    if os.name == "nt" and path.drive.upper() != "D:":
        raise ValueError("Project-controlled output/cache must remain physically on D:")
    return path


def load_manifest(path: Path) -> dict:
    manifest = json.loads(path.read_text(encoding="utf-8"))
    if manifest["schema"] != "arturia.minifreak-v.windows-shared-images.v1":
        raise ValueError("Unexpected shared asset schema")
    package = manifest["package"]
    if (package["url"], package["bytes"], package["sha256"]) != (
        PACKAGE_URL, PACKAGE_BYTES, PACKAGE_SHA256
    ):
        raise ValueError("Source package identity changed")
    names = set()
    for entry in manifest["resources"]:
        relative = PurePosixPath(entry["path"])
        if (relative.is_absolute() or ".." in relative.parts or "\\" in entry["path"]
                or ":" in entry["path"] or relative.as_posix() != entry["path"]):
            raise ValueError("Unsafe source asset path")
        folded = entry["path"].lower()
        if folded in names:
            raise ValueError("Ambiguous source asset case")
        names.add(folded)
        if (len(entry["sha256"]) != 64
                or any(c not in "0123456789abcdef" for c in entry["sha256"])
                or not isinstance(entry["bytes"], int) or entry["bytes"] < 0
                or not isinstance(entry["runtime"], bool)):
            raise ValueError("Invalid source asset record")
    if len(names) != manifest["source_file_count"]:
        raise ValueError("Source asset count changed")
    return manifest


def package_file(cache: Path, supplied: Path | None) -> Path:
    if supplied is not None:
        package = supplied.resolve()
    else:
        cache.mkdir(parents=True, exist_ok=True)
        package = cache / "MiniFreak_V__5_1_1_6945.exe"
        if not package.exists():
            partial = package.with_suffix(".exe.partial")
            offset = partial.stat().st_size if partial.exists() else 0
            if offset > PACKAGE_BYTES:
                raise ValueError("Preserve oversized partial package")
            if offset < PACKAGE_BYTES:
                headers = {"Range": f"bytes={offset}-"} if offset else {}
                request = urllib.request.Request(PACKAGE_URL, headers=headers)
                with urllib.request.urlopen(request, timeout=60) as response:
                    if offset:
                        expected = f"bytes {offset}-{PACKAGE_BYTES - 1}/{PACKAGE_BYTES}"
                        if response.status != 206 or response.headers.get("Content-Range") != expected:
                            raise ValueError("Partial package retained; server did not honor exact resume")
                    elif response.status != 200 or int(response.headers["Content-Length"]) != PACKAGE_BYTES:
                        raise ValueError("Package response identity changed")
                    with partial.open("ab" if partial.exists() else "xb") as stream:
                        remaining = PACKAGE_BYTES - offset
                        while remaining:
                            block = response.read(min(1024 * 1024, remaining))
                            if not block:
                                raise ValueError("Incomplete package retained for exact resume")
                            stream.write(block)
                            remaining -= len(block)
                        if response.read(1):
                            raise ValueError("Package response exceeded the pinned byte count")
            if partial.stat().st_size != PACKAGE_BYTES or digest(partial) != PACKAGE_SHA256:
                raise ValueError("Downloaded package failed identity check; partial retained")
            partial.rename(package)
    if package.stat().st_size != PACKAGE_BYTES or digest(package) != PACKAGE_SHA256:
        raise ValueError("Preserve existing package with unexpected identity")
    return package


def verify(root: Path, resources: list[dict]) -> None:
    root = root.resolve()
    for entry in resources:
        path = root.joinpath(*PurePosixPath(entry["path"]).parts)
        if not path.resolve().is_relative_to(root):
            raise ValueError("Asset escaped its verified root")
        if path.stat().st_size != entry["bytes"] or digest(path) != entry["sha256"]:
            raise ValueError("Source asset mismatch: " + entry["path"])


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=Path(__file__).resolve().parents[1]
                        / "resources/windows-shared-assets.json")
    parser.add_argument("--verified-root", type=Path,
                        help="Reuse extracted Shared/images bytes; every file is still checked")
    parser.add_argument("--installer", type=Path)
    parser.add_argument("--innoextract", type=Path)
    parser.add_argument("--cache", type=Path)
    parser.add_argument("--output", type=Path,
                        help="Dedicated generated asset directory; source files are never converted")
    parser.add_argument("--verify-only", action="store_true")
    args = parser.parse_args()
    manifest = load_manifest(args.manifest)
    if args.verified_root is not None:
        root = args.verified_root.resolve()
    else:
        if args.cache is None:
            parser.error("--cache is required when extracting the official package")
        cache = output_path(args.cache)
        package = package_file(cache, args.installer)
        extraction = cache / "windows-shared-5.1.1"
        extraction.mkdir(parents=True, exist_ok=True)
        tool = args.innoextract or shutil.which("innoextract")
        if tool is None:
            parser.error("Provide --innoextract; the official installer will not be executed")
        env = dict(os.environ)
        temporary = cache / "tmp"
        temporary.mkdir(exist_ok=True)
        env.update(TEMP=str(temporary), TMP=str(temporary), TMPDIR=str(temporary))
        command = [str(tool), "--extract", "--progress=0", "--color=0", "--output-dir",
                   str(extraction), "--include", os.path.join(*MEMBER_PARTS), str(package)]
        with (cache / "shared-images-extraction.log").open("w", encoding="utf-8") as log:
            subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, env=env, check=True)
        root = extraction.joinpath(*MEMBER_PARTS)
    verify(root, manifest["resources"])
    runtime = [entry for entry in manifest["resources"] if entry["runtime"]]
    if not args.verify_only:
        if args.output is None:
            parser.error("--output is required unless --verify-only is selected")
        output = output_path(args.output)
        output.mkdir(parents=True, exist_ok=True)
        expected = {entry["path"] for entry in runtime}
        for existing in output.rglob("*"):
            if existing.is_file() and existing.relative_to(output).as_posix() not in expected:
                raise ValueError("Preserve unexpected generated asset: " + str(existing))
        for entry in runtime:
            relative = PurePosixPath(entry["path"])
            target = output.joinpath(*relative.parts)
            if not target.resolve().is_relative_to(output):
                raise ValueError("Asset escaped generated output")
            target.parent.mkdir(parents=True, exist_ok=True)
            if target.exists():
                if target.stat().st_size != entry["bytes"] or digest(target) != entry["sha256"]:
                    raise ValueError("Preserve different generated asset: " + entry["path"])
            else:
                shutil.copyfile(root.joinpath(*relative.parts), target)
        verify(output, runtime)
    print(json.dumps({"source_files_verified": len(manifest["resources"]),
                      "runtime_files": len(runtime),
                      "runtime_bytes": sum(entry["bytes"] for entry in runtime),
                      "package_sha256": PACKAGE_SHA256,
                      "mode": "verify-only" if args.verify_only else "staged exact bytes"}))


if __name__ == "__main__":
    main()
