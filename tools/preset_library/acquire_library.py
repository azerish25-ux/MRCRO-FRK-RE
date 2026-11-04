"""Acquire the exact MiniFreak library from pinned public Arturia payloads.

Standard library only; no installer/account. Managed roots must be exclusively
owned by this build while it runs. Preexisting reparse/symlink and multiply-linked
managed files are rejected. This is not a sandbox against a privileged actor
concurrently replacing directories. Temporary files are fresh/exclusive; only
this invocation's still-identical temporary files are cleaned up.
"""
from pathlib import Path
from collections import Counter
import argparse
import gzip
import hashlib
import json
import os
import re
import stat
import tempfile
import time
import unicodedata
import urllib.request
import urllib.parse
import struct
import zlib

REPORT_NAME = 'library-acquisition-result.json'
REPARSE_POINT = getattr(stat, 'FILE_ATTRIBUTE_REPARSE_POINT', 0x400)
RESERVED = {'CON', 'PRN', 'AUX', 'NUL', 'CLOCK$', 'CONIN$', 'CONOUT$'}
RESERVED |= {prefix + suffix for prefix in ('COM', 'LPT')
             for suffix in '0123456789\u00b9\u00b2\u00b3'}


def digest(path):
    """Read-only convenience hasher; managed-file reads use identity checks too."""
    h = hashlib.sha256()
    with path.open('rb') as stream:
        while block := stream.read(1048576):
            h.update(block)
    return h.hexdigest()


def exact(stream, count):
    data = stream.read(count)
    if len(data) != count:
        raise ValueError('Truncated package stream')
    return data


def retry_os(operation):
    for attempt in range(2):
        try:
            return operation()
        except OSError:
            if attempt:
                raise
            time.sleep(5)


def lstat_optional(path):
    def get():
        try:
            return path.lstat()
        except FileNotFoundError:
            return None
    return retry_os(get)


def identity(info):
    return info.st_dev, info.st_ino


def check_info(path, info, directory):
    if stat.S_ISLNK(info.st_mode) or getattr(info, 'st_file_attributes', 0) & REPARSE_POINT:
        raise ValueError('Reparse/symlink path is not managed: ' + str(path))
    expected = stat.S_ISDIR if directory else stat.S_ISREG
    if not expected(info.st_mode):
        raise ValueError('Unexpected filesystem type: ' + str(path))
    if not directory and info.st_nlink != 1:
        raise ValueError('Multiply-linked file is not managed: ' + str(path))


def canonical_name(name):
    return unicodedata.normalize('NFC', unicodedata.normalize('NFC', name).casefold())


def portable_component(name):
    if not isinstance(name, str) or not name or name in ('.', '..'):
        raise ValueError('Empty/dot destination component')
    encoded = name.encode('utf-8')
    if len(encoded) > 255 or len(name.encode('utf-16-le')) // 2 > 255:
        raise ValueError('Destination component is too long')
    if any(ord(c) < 32 or ord(c) == 127 or c in '<>:"/\\|?*' for c in name):
        raise ValueError('Invalid portable destination character')
    if name.endswith(('.', ' ')):
        raise ValueError('Trailing destination dot/space')
    if name.split('.', 1)[0].rstrip(' .').upper() in RESERVED:
        raise ValueError('Reserved device destination component')
    return name


def portable_parts(raw):
    if not isinstance(raw, str):
        raise ValueError('Destination path must be a string')
    # Deliberately validate BEFORE Path/PurePath can normalize empty/dot parts.
    parts = tuple(portable_component(part) for part in raw.split('/'))
    if len(raw.encode('utf-8')) > 4095:
        raise ValueError('Destination path is too long')
    return parts


def validate_namespace(paths):
    files, directories, spellings = set(), set(), {}
    for parts in paths:
        key = tuple(canonical_name(p) for p in parts)
        for n in range(1, len(parts) + 1):
            prefix, spelling = key[:n], parts[:n]
            if prefix in spellings and spellings[prefix] != spelling:
                raise ValueError('Portable case/Unicode path alias')
            spellings[prefix] = spelling
        if key in files or key in directories or any(key[:n] in files for n in range(1, len(key))):
            raise ValueError('Duplicate/file-directory destination conflict')
        files.add(key)
        directories.update(key[:n] for n in range(1, len(key)))


class SafeTree:
    """Validated named workspace, with no reusable write-through file paths."""
    def __init__(self, raw_root):
        lexical = Path(os.path.abspath(os.fspath(raw_root)))
        current = Path(lexical.anchor)
        for part in lexical.parts[1:]:
            portable_component(part)
            current /= part
            info = lstat_optional(current)
            if info is not None:
                check_info(current, info, True)
        info = lstat_optional(Path(lexical.anchor))
        if info is not None:
            check_info(Path(lexical.anchor), info, True)
        resolved = retry_os(lambda: lexical.resolve(strict=False))
        if os.path.normcase(str(resolved)) != os.path.normcase(str(lexical)):
            raise ValueError('Root resolves through a filesystem alias')
        self.root = resolved
        self.root_identity = None
        self.check_root()

    def check_root(self):
        current = Path(self.root.anchor)
        for part in self.root.parts[1:]:
            current /= part
            info = lstat_optional(current)
            if info is not None:
                check_info(current, info, True)
        info = lstat_optional(self.root)
        if info is not None:
            check_info(self.root, info, True)
            if self.root_identity is not None and identity(info) != self.root_identity:
                raise ValueError('Managed root identity changed')
            self.root_identity = identity(info)
        elif self.root_identity is not None:
            raise ValueError('Managed root disappeared')

    def path(self, parts, create_parents=False):
        self.check_root()
        current = self.root
        if create_parents:
            retry_os(lambda: current.mkdir(parents=True, exist_ok=True))
            self.check_root()
        for index, part in enumerate(parts):
            portable_component(part)
            info = lstat_optional(current)
            if info is not None:
                check_info(current, info, True)
                for entry in retry_os(lambda: list(os.scandir(current))):
                    if canonical_name(entry.name) == canonical_name(part) and entry.name != part:
                        raise ValueError('Existing portable spelling alias: ' + str(current / entry.name))
            current /= part
            last = index == len(parts) - 1
            info = lstat_optional(current)
            if info is None and not last and create_parents:
                retry_os(lambda: current.mkdir(exist_ok=True))
                info = lstat_optional(current)
            if info is not None:
                check_info(current, info, not last)
            resolved = retry_os(lambda: current.resolve(strict=False))
            if not resolved.is_relative_to(self.root) or resolved != current:
                raise ValueError('Destination resolves through an alias/outside root')
        return current

    def files(self):
        self.check_root()
        if lstat_optional(self.root) is None:
            return set(), set()
        files, directories = set(), set()
        pending = [self.root]
        while pending:
            directory = pending.pop()
            for entry in retry_os(lambda: list(os.scandir(directory))):
                path = Path(entry.path)
                info = lstat_optional(path)
                if info is None:
                    raise ValueError('Workspace changed during validation')
                if stat.S_ISDIR(info.st_mode):
                    check_info(path, info, True)
                    directories.add(path.relative_to(self.root).as_posix())
                    pending.append(path)
                else:
                    check_info(path, info, False)
                    files.add(path.relative_to(self.root).as_posix())
        return files, directories

    def matches(self, parts, size, sha):
        path = self.path(parts)
        expected = lstat_optional(path)
        if expected is None or expected.st_size != size:
            return False
        flags = os.O_RDONLY | getattr(os, 'O_BINARY', 0) | getattr(os, 'O_NOFOLLOW', 0)
        fd = retry_os(lambda: os.open(path, flags))
        with os.fdopen(fd, 'rb') as stream:
            opened = os.fstat(stream.fileno())
            check_info(path, opened, False)
            if identity(opened) != identity(expected):
                raise ValueError('Managed file changed while opening')
            h = hashlib.sha256()
            while block := stream.read(1048576):
                h.update(block)
        self.path(parts)
        return h.hexdigest() == sha


def cleanup_owned_temp(path, created):
    """Never delete a preexisting/replaced path, even during error cleanup."""
    info = lstat_optional(path)
    if info is not None and identity(info) == created:
        if stat.S_ISREG(info.st_mode) and not (getattr(info, 'st_file_attributes', 0) & REPARSE_POINT):
            path.unlink()


def atomic_write(tree, parts, writer, size, sha):
    for attempt in range(2):
        temporary = created = None
        try:
            if tree.matches(parts, size, sha):
                return
            target = tree.path(parts, create_parents=True)
            # mkstemp uses random names and O_CREAT|O_EXCL, never a prior .part.
            fd, name = tempfile.mkstemp(prefix='.mfrk-acquire-', suffix='.tmp', dir=target.parent)
            temporary = Path(name)
            created = identity(os.fstat(fd))
            with os.fdopen(fd, 'wb') as stream:
                tree.path(parts)  # Recheck parents before sending any data.
                writer(stream)
                stream.flush()
                os.fsync(stream.fileno())
            info = lstat_optional(temporary)
            if info is None or identity(info) != created:
                raise ValueError('Temporary file identity changed')
            check_info(temporary, info, False)
            if info.st_size != size or digest(temporary) != sha:
                raise ValueError('Completed temporary hash/size mismatch')
            target = tree.path(parts)
            os.replace(temporary, target)
            temporary = None
            if not tree.matches(parts, size, sha):
                raise ValueError('Committed output identity mismatch')
            return
        except OSError:
            if attempt:
                raise
            time.sleep(5)
            # An uncertain replace may already have committed; check before retry.
            if tree.matches(parts, size, sha):
                return
        finally:
            if temporary is not None:
                retry_os(lambda: cleanup_owned_temp(temporary, created))


def save_exact(target, data, sha, root=None):
    target = Path(os.path.abspath(os.fspath(target)))
    tree = SafeTree(target.parent if root is None else root)
    if not target.is_relative_to(tree.root):
        raise ValueError('Destination outside managed root')
    parts = portable_parts(target.relative_to(tree.root).as_posix())
    atomic_write(tree, parts, lambda stream: stream.write(data), len(data), sha)


def response(url, start, length, total):
    end = start + length - 1
    request = urllib.request.Request(url, headers={
        'Range': f'bytes={start}-{end}', 'User-Agent': 'MiniFreak-Library-Verification/1.0'})
    result = urllib.request.urlopen(request, timeout=60)
    if result.status != 206 or result.headers.get('Content-Range') != f'bytes {start}-{end}/{total}':
        result.close()
        raise ValueError('Unexpected package range response')
    return result


def preflight(m, output, cache):
    if m['schema'] != 'arturia.minifreak-v.complete-preset-library.v1':
        raise ValueError('Manifest schema')
    if len(m['files']) != m['total_runtime_files'] or dict(Counter(r['role'] for r in m['files'])) != m['expected_role_counts']:
        raise ValueError('Incomplete library manifest')
    if urllib.parse.urlsplit(m['package_url']).scheme != 'https':
        raise ValueError('Pinned package must use HTTPS')
    packages, cache_names = {}, {}
    for payload in m['payloads']:
        label = portable_component(payload['package'])
        name = portable_component(label.replace(' ', '_') + '.payload.cpio.gz')
        if label in packages or canonical_name(name) in cache_names:
            raise ValueError('Duplicate/aliased payload cache key')
        if not re.fullmatch('[0-9a-f]{64}', payload['sha256']):
            raise ValueError('Invalid payload digest')
        if type(payload['range_start']) is not int or type(payload['range_length']) is not int or payload['range_start'] < 0 or payload['range_length'] <= 0 or payload['range_start'] + payload['range_length'] > m['package_total_bytes']:
            raise ValueError('Invalid payload range')
        packages[label] = payload
        cache_names[canonical_name(name)] = name
    wanted, outputs, plans, referenced = {}, set(), [], set()
    for spec in m['files']:
        role = portable_component(spec['role'])
        parts = portable_parts(spec['output_path'])
        relative = portable_parts(spec['path'])
        if len(parts) < 2 or parts[0] != role or parts[1:] != relative:
            raise ValueError('Output role/path metadata mismatch')
        key = spec['payload'], spec['member_path']
        if key in wanted or spec['payload'] not in packages:
            raise ValueError('Duplicate/unknown source payload member')
        for field in ('sha256', 'package_sha256'):
            if not re.fullmatch('[0-9a-f]{64}', spec[field]):
                raise ValueError('Invalid member digest')
        if any(type(spec[field]) is not int or spec[field] < 0 for field in ('bytes', 'package_bytes')):
            raise ValueError('Invalid member size')
        if spec['transform'] not in ('identity', 'final_lf_to_crlf', 'lf_to_crlf'):
            raise ValueError('Unknown pinned transformation')
        plans.append(parts)
        outputs.add(spec['output_path'])
        wanted[key] = spec
        referenced.add(spec['payload'])
    validate_namespace(plans)
    if referenced != set(packages):
        raise ValueError('Unused source payload')
    output_tree, cache_tree = SafeTree(output), SafeTree(cache)
    left, right = output_tree.root, cache_tree.root
    if left == right or left.is_relative_to(right) or right.is_relative_to(left):
        raise ValueError('Output/cache roots overlap')
    left_info, right_info = lstat_optional(left), lstat_optional(right)
    if left_info is not None and right_info is not None and identity(left_info) == identity(right_info):
        raise ValueError('Output/cache root alias')
    for parts in plans:
        output_tree.path(parts)
    for name in [*cache_names.values(), REPORT_NAME]:
        cache_tree.path((name,))
    actual_files, actual_dirs = output_tree.files()
    expected_dirs = {'/'.join(parts[:n]) for parts in plans for n in range(1, len(parts))}
    if not actual_files <= outputs or not actual_dirs <= expected_dirs:
        raise ValueError('Unexpected existing output entries')
    return output_tree, cache_tree, packages, wanted, outputs


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, default=Path(__file__).with_name('acquisition-manifest.json'))
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cache', type=Path, required=True)
    args = parser.parse_args()
    m = json.loads(args.manifest.read_text(encoding='utf-8-sig'))
    output_tree, cache_tree, packages, wanted, outputs = preflight(m, args.output, args.cache)
    with response(m['package_url'], 0, m['package_xar_heap_offset'], m['package_total_bytes']) as stream:
        prefix = exact(stream, m['package_xar_heap_offset'])
    magic, size, version, compressed, uncompressed, algorithm = struct.unpack('>4sHHQQI', prefix[:28])
    toc = zlib.decompress(prefix[size:])
    if magic != b'xar!' or size != 28 or version != 1 or size + compressed != m['package_xar_heap_offset'] or len(toc) != uncompressed or hashlib.sha256(toc).hexdigest() != m['decompressed_toc_sha256']:
        raise ValueError('Package TOC changed')
    found, proofs = {}, []
    for payload in m['payloads']:
        cache_parts = (payload['package'].replace(' ', '_') + '.payload.cpio.gz',)
        def download(target):
            with response(m['package_url'], payload['range_start'], payload['range_length'], m['package_total_bytes']) as source:
                while block := source.read(1048576):
                    target.write(block)
        atomic_write(cache_tree, cache_parts, download, payload['range_length'], payload['sha256'])
        file = cache_tree.path(cache_parts)
        proofs.append({'package': payload['package'], 'sha256': digest(file), 'bytes': file.stat().st_size})
        with gzip.open(file, 'rb') as source:
            while True:
                header = exact(source, 76)
                if header[:6] != b'070707':
                    raise ValueError('Unexpected CPIO profile')
                mode, name_size, length = int(header[18:24], 8), int(header[59:65], 8), int(header[65:76], 8)
                if not 1 <= name_size <= 1048576:
                    raise ValueError('CPIO name bound')
                name = exact(source, name_size)
                if name[-1:] != b'\0':
                    raise ValueError('Unterminated CPIO name')
                name = name[:-1].decode('utf-8')
                if name == 'TRAILER!!!':
                    break
                key = payload['package'], name
                if key in wanted and mode & 0o170000 == 0o100000:
                    spec = wanted[key]
                    if spec['output_path'] in found or length != spec['package_bytes']:
                        raise ValueError('Member count/size mismatch')
                    data = exact(source, length)
                    if hashlib.sha256(data).hexdigest() != spec['package_sha256']:
                        raise ValueError('Member source hash mismatch')
                    if spec['transform'] == 'final_lf_to_crlf':
                        if not data.endswith(b'\n'):
                            raise ValueError('Expected final LF')
                        data = data[:-1] + b'\r\n'
                    elif spec['transform'] == 'lf_to_crlf':
                        data = data.replace(b'\r\n', b'\n').replace(b'\n', b'\r\n')
                    if len(data) != spec['bytes'] or hashlib.sha256(data).hexdigest() != spec['sha256']:
                        raise ValueError('Installed identity mismatch: ' + spec['output_path'])
                    atomic_write(output_tree, portable_parts(spec['output_path']), lambda target: target.write(data), len(data), spec['sha256'])
                    found[spec['output_path']] = spec['sha256']
                else:
                    while length:
                        block = exact(source, min(length, 1048576))
                        length -= len(block)
    if set(found) != outputs or output_tree.files()[0] != outputs:
        raise ValueError('Missing/unexpected staged library files')
    report = {'runtime_files': len(found), 'role_counts': m['expected_role_counts'], 'preset_archive_count': m['preset_archive_count'], 'browser_catalog_count': m['browser_catalog_preset_count'], 'tutorial_state_count': m['tutorial_state_count'], 'hardware_backup_count': m['hardware_backup_preset_count'], 'verified_bytes': m['runtime_bytes'], 'payloads': proofs, 'authentication_used': False, 'installer_executed': False, 'all_original_identities_verified': True, 'file_hashes': found}
    data = (json.dumps(report, indent=2) + '\n').encode('utf-8')
    atomic_write(cache_tree, (REPORT_NAME,), lambda target: target.write(data), len(data), hashlib.sha256(data).hexdigest())
    print(json.dumps({key: value for key, value in report.items() if key not in ('file_hashes', 'payloads')}, indent=2))


if __name__ == '__main__':
    main()
