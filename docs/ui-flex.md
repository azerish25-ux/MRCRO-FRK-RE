# Native Flex layout solver

`mfrk::ui::flex` implements the original pure numeric Flex calculation and
explicit nested-box recursion. It extends the strict-FP `mfrk_ui` static library.

`layout` returns float rectangles in original item storage order.
`layout_tree` accepts a caller-owned value-record graph, borrowed for the call,
and returns rectangles for each node. Children use indices, with no native
pointer crossing the API. Member initializers are adapter conveniences; they
do not establish original XML or factory defaults.

## Original behavior retained

Calculation state uses double, followed by original float bounds stores.
Equal-order items retain storage order. Shrink factors are additive; line totals
remain accumulated across freeze retries. Wrapping uses strict overflow.
Reverse directions add the float endpoint before promoting it to double.
Space-evenly uses count+1, and auto margins, negative margins, conflicting
constraints and signed target rectangles retain the original behavior.

This calculation-state precision does not define every GUI cache's type.
Runtime layout, property/default construction and resize callbacks are separate
source-bound stages.

## Admission

Box enums and item fields are declared in `mfrk/ui_flex.hpp`. Inputs must meet
the finite, bounded and sentinel contracts: at most256 items per box,256 nodes,
absolute float inputs at most1,000,000; dimensions/basis nonnegative or-1;
nonnegative grow/shrink; signed margins with exactly-2 meaning auto.

Trees are rooted at0, topologically indexed, with exactly one parent per
nonroot. Shared, cyclic and unreachable nodes reject. Every derived child area
must meet the same bounds. Inputs remain owned and stable for the duration of
the call; returned vectors own their results.

Nearest IEEE binary32/binary64, gradual underflow, masked exceptions and strict
operations without contraction/reassociation are required. Exception flags,
trap behavior and allocation failures are outside the profile. Discarded
auto-margin division intermediates may be nonfinite. The public solver returns
float rectangles; integer edge conversion remains a separate admitted primitive.

## Validation

Independent original-instruction reproduction regenerated both gold files
byte-identically:4,964 flat boxes/35,802 item rectangles and144 nested trees/
720 boxes/2,880 rectangles. Two complete function bodies match the reviewed
candidate exactly. Additional original observations cover72 conflicting-
constraint/signed-area cases and8 trees up to16 levels deep; the actual compiled
library matches all216+171 supplemental rectangles.

The integrated host Release suite passes40 checks. Private fixture readers
check complete size/CRC32/SHA256 before parsing and validate all cases before
numeric execution. Fourteen admission/conversion guards and14 altered files
reject, including same-CRC/different-SHA data. All12 semantic mutants reject.
Source and canonical fixture identities are recorded in
`core/tests/ui_flex_provenance.json`.
[Run41920532205](https://github.com/azerish25-ux/MRCRO-FRK-RE/actions/runs/41920532205)
passes all40 Linux Release checks and the flat/nested/guard references on an
API35 x86_64 Android emulator. Both Android ABIs compile; arm64 runtime remains
unverified. [Exact target record](ui-flex-validation.json) pins the source and jobs.

The original instruction oracle modeled owned allocator/byte-copy boundaries;
original numeric helpers, sorting, recursion and stack/cookie instructions ran.
Associated Component pointers were null; nested FlexBox pointers referenced
owned initialized nodes. This proves the bounded numeric calculation, without
Component setter clamps/callbacks, XML/scene interpretation, fonts, painting,
rasterization, pointer lifecycle or full UI parity.
