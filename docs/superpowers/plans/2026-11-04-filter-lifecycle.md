# Deferred fixed-model49 filter lifecycle

Implement original valid public-Init →prepare/render/end lifecycle, not a
synchronous replacement approximation. Keep one logical raw state across resources,
preserve PRNG, reconstruct original cache/dirty conversion and reset-before-
pending replay. Both numeric kernels and factor selection/fade are already verified.

- [ ] Integrate exact seven-record normalized/physical cache semantics and tests.
- [ ] Add raw rate-initialization/reset preserving mode/noise/seed.
- [ ] Implement explicit prepare/render/end and worker/completion phases with
  bounded mailboxes and pending latest-value normalized updates.
- [ ] Verify original cold start, first-context branches, deferred factor1/3/4
  transitions, suspended no-write/PRNG freeze, pending reset order and metadata.
- [ ] Cross-kernel and repeated-request evidence/validation where supported.
- [ ] Independent review, remote Android tests, integrate and continue full graph.

Constructor-only original audio is invalid despite ready=true/null resource.
The portable API requires initialize before prepare/render. context+50 retains
an explicit direct-build branch name; no unproven offline/realtime label.
Render and end are separate, matching native child/owner flag clearing.

All control calls belong to the audio/controller actor. Worker step is a separate
off-callback phase; queue publication synchronizes numeric resource/raw init.
The port builds a private generation preserving the old seed/latches, then
installs it while rendering is suspended; retired generations die on the worker.
This avoids sharing mutable C++ raw state with the audio actor. It preserves the
original reset results/order without claiming Windows pointer identity.
Physical-origin configuration is serialized outside a running worker. Actual OS
timing/ownership and arbitrary state-command snapshots remain unverified; do not
claim their equivalence from controlled schedule tests. Raw model49 flag is fixed
true in this node, as the actual Dual child constructor wires it; hardware37 is
the separate Dual child, not an invented reset of this node.

Source: local/evidence/filter-agent/FILTER_WRAPPER_HANDOFF.md and
NORMALIZED_PARAMETER_HANDOFF.md, original transition/cold/pending fixtures.
All files/temp onD:,5-second storage retry; Android tools/emulators onlyActions.
