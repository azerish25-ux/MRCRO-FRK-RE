# Pointer coordinates and hit resolution

`ui_pointer_geometry.hpp` implements original `108d350/108dbe0/10911f0/108da30`,
`f81c50`, `ff4ed0/ff5210` and `ffa390/ffa210`. The pointer owner can now use
`GuiMouseGeometryBinding` for actual ancestor coordinate conversion, clipped
containment and reverse paint-order child hit traversal.

The implementation retains source binary32 operation order, affine inversion
(including the original singular-matrix return), root/peer/Desktop scaling and
the original binary64-add/low-word point rounding. Hit lookup divides by scale;
local event conversion multiplies by a rounded reciprocal. These paths must not
be merged into a generic scaling helper. Null endpoints represent Desktop/global
logical space. Invalid peers clear the pointer owner's peer slot.

The original executable comparison covers 560 cases and 1,030 service events,
with zero local differences. It includes five-node trees, common/foreign roots,
overlapping children, hidden children, clipping, missing peers, half-point edges,
scales and affine matrices. Exact fixture/producer identities are recorded in
`core/tests/pointer_geometry_provenance.json`. Native and Android workflow checks
passed in native run44070901499 and renderer run44070901589. Exact identities
are recorded in `validation/pointer-geometry-android-20261117.json`.

This admission requires strict nearest floating point with contraction disabled,
finite scales/coordinates/matrices/intermediates, representable rounded points,
nonnegative bounds and acyclic complete component trees. Child storage and peer
implementations survive each traversal. Platform peer mapping/validity/contains,
component scale and virtual hit-test endpoints remain actual application services.

The six queued Android pointer scenarios now exercise this production geometry
binding over a real small component tree. Their platform peer and component
event endpoints remain observers. This does not establish the MiniFreak scene,
physical touch binding for all widgets, raster, sound, MIDI or preset parity.
