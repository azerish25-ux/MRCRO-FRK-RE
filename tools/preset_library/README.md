# Included preset and dependency acquisition

`acquire_library.py --manifest acquisition-manifest.json --output <owned-root>
--cache <owned-cache>` reproduces the installed global MiniFreak library from
three pinned public Arturia package payloads. It verifies XAR, payload, member
and final Windows file identities without executing an installer or using an
account. Every managed root must be exclusively owned by this build.

The1380runtime files preserve626browser presets,72tutorial states,512hardware
backup slots(including256emptyInitfiles),82samples,79wavetables,sixbankcovers
andthreeindex/whitelist files. File and slot identities remain distinct. Presets
contain counted binary blobs inside text archives; transformations are pinned
per-file. They never globally normalize archive newlines.

Managed paths reject aliases/traversal/device/ADS names, unsafe links and planned
collisions. Writes use fresh exclusive same-directory temporary files and atomic
replacement; an existing `.part` file is never opened or cleaned up. This producer
assumes exclusive build-owned roots, rather than acting as a sandbox against a
privileged concurrent directory replacement.

The remote workflow checks the hardened writer, including four real file-symlink
cases that could not execute under the local Windows token. It acquires and
verifies all1380files twice, comparing deterministic reports. Only validation
reports are uploaded; original staged library files and package caches remain
outside source git and artifact inputs. No local Android tools are involved.

`producer-provenance.json` retains reviewed and public normalized source hashes.
The public Python AST and parsed manifest are identical to the independently
reviewed revision. The test harness changes only sibling paths and explicit
work/report locations.

This is the global library asset producer. Native loader migration, component
FX XML/behavior, remaining application resources and complete instrument sound
are separate integration work. Acquired original bytes do not establish their
interpretation or audio equivalence.
