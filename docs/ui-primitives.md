# Original UI numeric primitives

`mfrk::ui` supplies seven source-derived mathematical primitives in a separate
static library. It does not render Home, Advanced or Sequencer screens.

| Primitive | Original cases |
| --- | ---: |
| Interpolated center bounds | 40 |
| Packed sine/cosine return lanes | 37,056 |
| Outline gradient endpoints | 40 |
| Center value marker points | 80 |
| Magnetic drag mapping | 648 |
| Flex integer edge projection | 270 |
| Scaled absolute dimension | 176 |

The matched original wrapper SHA-256 is
`354c26c58e4c8c671b50f88d070452b47c9cd6b6505b1ea18391724929b3d19e`.
All ten numerical bodies, including three private helpers, preserve the reviewed
candidate text exactly. The public namespace and implementation linkage changed;
the public packed-pair function is named `packed_sine_cosine`.

The implementation compiles separately with strict floating-point options, PIC
enabled and interprocedural optimization disabled. It requires nearest IEEE
binary32/binary64 operations, gradual underflow and masked FP exceptions.
Caller inputs and intermediate calculations must satisfy the public admission
contracts. This is not an arbitrary input/state parser.

## Behavior that must survive integration

Center radius uses integer half dimensions while center coordinates use float
halves. Gradient angle is multiplied by the original binary32 pi. Marker
distances preserve the original start/width interpretation and radius factor.
The packed polynomial preserves original return bits instead of using host
trigonometry. Magnetic mapping retains plateau midpoints, gaps, nearest ties,
point zones and explicit skip flags, without adding wrapping or result clamping.
Flex quantizes individual endpoints before subtracting. Absolute dimension
scaling multiplies promoted binary32 scale by binary64 size before adding0.5
and truncating.

These are bounded original instruction branches: explicit center transition,
gradient enabled/follow-value false, marker after the parameter getter,
matching-parameter/non-wrapping magnetic mapping, pre-setBounds Flex projection
and scaled-absolute dimensions. They do not infer animation, hover predicates,
color/default producers, layout policy, pointer capture, fonts or rasterization.

## Evidence and verification

An independent review reran all seven unchanged original instruction probes and
reproduced every canonical fixture byte. Thirty additional native geometry cases
and1,728 magnetic mappings also match the production API. The ten moved bodies
were independently extracted and compared.

The integrated Release host suite passes37 checks. UI reference files are
bounded and checked against exact size, CRC32 and SHA256 before numeric payload
decoding. Fifteen direct domain/boolean guards,49 altered files including
same-CRC/different-SHA cases, and seven semantic mutants reject. These checks
exercise trusted test infrastructure; they add no behavior to the production
functions.

Provenance is in `core/tests/ui_primitives_provenance.json`. Original captures,
source slices, independent fresh instruction outputs and detailed review
artifacts remain on D:. Android compilation and runtime verification are pending
for this candidate. Full rendering, scene interpretation, touch gestures and UI
equivalence remain required. The existing three-knob comparison application's
Canvas conventions are still unverified.
