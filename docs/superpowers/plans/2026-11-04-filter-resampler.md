# Original filter resampling and settled chain

Goal: implement the original eight-section IIR up/downsampler for factors2/4/6/8
and connect both audio/modulation inputs to model49 at the actual internal rate.
This is a required stage of the full filter wrapper. Asynchronous resource
replacement, fade transitions and dual switching remain separate unfinished work.

Files: core/include/mfrk/iir_resampler.hpp, core/src/iir_resampler.cpp and exact
coefficient header; core/include/mfrk/oversampled_vcf49.hpp and corresponding
source; original binary fixtures/generator/test, CMake and remote Android workflow.

- [x] Independent original setup/up/down/reset traces and mathematical recurrence.
- [x] Actual settled wrapper trace includes both input resamplers and raw49.
- [ ] Establish failing original-reference tests against silent stages.
- [ ] Implement coefficient/state/gain/decimation/reset semantics without FMA.
- [ ] Connect settled fixed-resource chain with startup-allocated buffers.
- [ ] Verify original IIR outputs/states and chain outputs on native/Android.
- [ ] Independent review and integration; continue complete wrapper and instrument.

Tests compare9,600 upsampled +1,920 downsampled values and60 paired state blocks,
then1,920 outputs from the actual original settled ModuleUpsampler execution.
Coefficients come from original static initializer descriptors. Full probe scripts,
method limitations and independent comparisons are local/evidence/filter-agent.

Keep all local artifacts on D:, retry storage access after5seconds. Android
toolchains/emulators run only on GitHub Actions. Do not change the live synth.
