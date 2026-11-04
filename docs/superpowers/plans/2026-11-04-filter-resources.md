# Complete numeric filter resources and deferred lifecycle

Preserve original selector, factor1 bypass, missing-key factor3 IIR fallback,
FIR phase/history math, vector fade and resource/controller ordering. These are
required dependencies of complete filter/voice reconstruction, not substitutes
for the full Android instrument.

- [ ] Extend IIR resource factors1/3 with actual bypass/fallback semantics.
- [ ] Implement original FIR profiles and startup-only convolution buffers.
- [ ] Share fixed-resource processing across IIR/FIR without callback allocation.
- [ ] Implement exact adaptive selector and32-sample four-lane fade.
- [ ] Verify original numeric output/state references locally and on Android.
- [ ] Implement deferred request/build/install/completion, pending normalized
  replay and reset-before-replay, including native cold-start contract.
- [ ] Verify transition schedules, then integrate full voice/cyclic/UI work.

Source: local/evidence/filter-agent/FILTER_WRAPPER_HANDOFF.md. Default adaptive
TRUE comes from full Dual constructor, superseding wrapper-only FALSE. Internal
IIR/FIR type is not the GUI LP/BP/HP mode. Factor3 fallback must preserve its
original zero-gain behavior; do not design substitute coefficients.

Files: filter_resampling.hpp/cpp (settings/fade), fir_resampler.hpp/cpp and source
tap profiles, filter_resource.hpp/cpp (both-input fixed-resource chain), existing
IIR/OversampledVcf49 adaptation, original fixtures/generator/test and Actions.
Explicit worker scheduling/ownership and nonempty command-state replay remain
separate from proven numeric construction until verified.

All artifacts stay on D:,5-second storage retry. Android tools/emulators only on
GitHub Actions. No original synth changes or new live synth instances.
