# Deferred fixed-model49 filter lifecycle

Implement original valid public-Init →prepare/render/end lifecycle, not a
synchronous replacement approximation. Keep one logical raw state across resources,
preserve PRNG, reconstruct original cache/dirty conversion and reset-before-
pending replay. Both numeric kernels and factor selection/fade are already verified.

- [x] Integrate exact seven-record normalized/physical cache semantics and tests.
- [x] Add raw rate-initialization/reset preserving mode/noise/seed.
- [x] Implement explicit prepare/render/end and worker/completion phases with
  bounded mailboxes and pending latest-value normalized updates.
- [x] Verify original cold start, first-context branches, deferred factor1/3/4
  transitions, suspended no-write/PRNG freeze, pending reset order and metadata.
- [x] Verify both cross-kernel transition directions and captured cache states.
- [x] Independent review/Android/integration at63ddbfe193e939d53b3a1df1848a3fa6f567d99a.
- [ ] Further repeated/overlapping requests, arbitrary captured-state replay and
  full graph/application requirements remain open.

Red41835908163 rejected placeholder controller with30,296 mismatches; other
checks passed. Green41836390093 passed all14 native checks, both Android ABI
builds and API35 x86_64 execution:1,398commands/24,064outputs/814states/1,834
parameter snapshots,6suspended checks,0callbackallocations,0mismatches.
Parameter gold:310traces+7constructor records+7metadata rows. Portable worker
allocation failure preserves the request and can retry; this is not a claim
about native Windows OOM behavior. Source/provenance files accompany fixtures.

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
