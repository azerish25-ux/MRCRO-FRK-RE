# Live scene construction orchestration

`ui_scene_factory.hpp/.cpp` implements the construction order recovered from
`9b1b50/9b1c40/9b2120`: structural walking invokes whole-node construction,
actual class creation precedes attachment, children build before the widget's
post-build callback, then the parent's dimensions and previous-child coordinates
are refreshed for the next sibling. Previous centers use integer division before
addition. Nested `components` and `params` preserve their special handling.

Generic component resolution retains the helper pass and the second pass only
when the result still says `component`. The source metadata tags do not create
widgets; `xmlfile_component` invokes the required include service. Other nodes
require the first explicit title attribute and a successful real constructor.
Missing titles, empty generic kinds and unknown constructors fail explicitly.

The factory's tag dispatch uses signed bytes in its 64-bit djb2 loop. Six values
were executed through original instructions `9b2a30..9b2a73`, including high bytes
and the actual empty-string extension branch at `9b920c`. The integration check
uses parsed XML, native-style geometry, nested/opaque components, a scoped alias,
an include callback changing parent dimensions, and a post-build resize. Its six
widgets and 19 ordered events verify the orchestration and existing geometry
join. This integration check is not a whole-original-factory comparison.

The required `Services` owns the actual widget constructors, generic style
resolution/alias/localization, source attachment/property/controller registration,
post-build methods, previous-child title lookup and include loading. No generic
widget fallback or fixed-position replacement is installed. These service bodies
still need their full application implementations; the comparison app has not
been changed into a complete synth scene by this addition.

Admission retains existing XML/scope/expression contracts, complete component
storage through construction, depth at most128, and representable signed32
sibling endpoints. Original timing counters and optional kind collection are
diagnostic and omitted. Allocation failures and exact exception text are outside
this contract. Full original construction/scene/event/raster parity is unproven.

Linux and Android run44076727823 passed the integration check. Exact revision,
jobs, runtime scope and remaining limits are recorded in
`validation/scene-factory-android-20261117.json`.
