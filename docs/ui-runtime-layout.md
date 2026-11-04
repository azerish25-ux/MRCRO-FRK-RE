# Source-derived runtime resize actor

`mfrk::ui::runtime` extends the native UI library with geometry producers,
original float round/trunc behavior, a direct C0 layout step and an admitted
owner-resize event protocol. It accepts caller-initialized typed state; member
initializers do not establish native factory or XML defaults.

Runtime items retain **float32 cached lengths and rounding carry**. Geometry
uses double storage. Initial relative geometry uses float reciprocal/multiply
then widening; initial absolute geometry converts integer bounds exactly to
double. Runtime geometry uses float division and inverse-scale multiplication
then widens both results. These producers are distinct.

All four native item bytes are represented: fixed, keep_ratio, opaque flag+A
and fixed_current+B. C0 ignores+A and uses+B for fixed extents. Parent fixed-cache
refresh selects current integer extent when+A or+B is true. Combined fixed/ratio
and nonzero ratio minima retain native behavior. The source owner entry forces
cross-axis fill even when the direct C0 option is false.

Changed child sizes deliver the source guarded refresh before cached-length
accumulation. A source-justified auto-owner resize performs one guarded self
reentry. This preserves the cache changes that narrower cases missed.
[Admission changes and source sites](ui-runtime-layout-delta.md) explain the
original branch proof and the unchanged v1 evidence.

## Scope and admission

Nearest IEEE binary32/binary64, gradual underflow, masked FP exceptions and
strict operations without reassociation/contraction are required. Public header
contracts and checked finite/representable ranges apply. State and inputs remain
owned by the caller for each call. Mutating operations provide a basic exception
guarantee, with no rollback of earlier updates.

The event model covers one ordinary observer and the observed guarded child
size-change delivery. External child-entry events, other observers/features,
weak references, queued/animated scheduling, general Component setter behavior,
rotation and threads remain separate. Positive finite background/view scales
and supported auto-owner extents are required. This is not a full scene or event
framework. The direct C0 entry performs no observer callbacks.

## Evidence and validation

Independent unchanged-instruction reproduction regenerated all nine canonical
corpora byte for byte. Five initial numerical bodies remain unchanged; the moved
C0 calculation and parent protocol have explicit source-proved deltas.

| Reference kind | Original cases | Item updates |
| --- | ---: | ---: |
| Initial geometry | 1,205 | — |
| Child resize | 1,608 | — |
| Runtime geometry | 1,203 | — |
| Direct C0 | 2,400 | 4,716 |
| Owner event | 1,002 | 2,031 |
| UCRT round/trunc pairs | 24,951 | — |
| Current-flag owner events | 1,728 | 3,595 |
| All-four-flag C0 | 1,312 | 2,643 |
| All-four-flag owner events | 3,616 | 7,251 |

All16 masks, visibility, scales, fixed/auto-owner branches, disabled state and
directed ratio-minimum cases are covered. The integrated Release host suite
passes50 checks. The isolated module passes10 checks,20 guards,63 altered-file
rejections (including nine same-CRC/different-SHA cases) and19 semantic mutants.
Three historical rejection controls demonstrate the invalid former exclusions.

Readers check full size/CRC32/SHA256 before payload interpretation, then bounded
LE records, booleans, complete decoding and EOF before numerical execution.
They validate trusted gold, without becoming a product interchange parser.
Provenance is in `core/tests/ui_runtime_layout_provenance.json`.
[Run41930772735](https://github.com/azerish25-ux/MRCRO-FRK-RE/actions/runs/41930772735)
passes all50 Linux Release checks and all9 reference corpora plus20 guards on
API35 x86_64 Android. Both ABIs compile; arm64 runtime remains unverified.
[Exact target results](ui-runtime-layout-validation.json) pin the source and jobs.
An independent Win64 call-frame audit replayed all4,016 geometry cases with
correct entry stack alignment and unchanged golden bytes;
[ABI audit](ui-runtime-oracle-abi.json) records the historical setup correction.
Full declarative scene, painting, fonts, rasterization, touch lifecycle and
standalone instrument parity remain required.
