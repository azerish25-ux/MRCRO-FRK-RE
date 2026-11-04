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
- [x] Establish failing original-reference tests against silent stages.
- [x] Implement coefficient/state/gain/decimation/reset semantics without FMA.
- [x] Connect settled fixed-resource chain with startup-allocated buffers.
- [x] Verify original IIR outputs/states and chain outputs on native/Android.
- [x] Independent review/integration at3a5b28aec7e9ddb2c82fdb88ec5120c020fdb4ed.
- [ ] Continue complete wrapper, envelopes and full instrument/application.

Red41816094920 rejected placeholders with15,336 combined mismatches, earlier
checks passed. Green41816365296 passed all10 native checks, both Android ABI
builds and x86_64 API35 emulator comparisons, with zero output/state mismatches.
Settled chain fixtures use96kHz host rate. arm64 execution remains unverified.

Tests compare9,600 upsampled +1,920 downsampled values and60 paired state blocks,
then1,920 outputs from the actual original settled ModuleUpsampler execution.
Coefficients come from original static initializer descriptors. Full probe scripts,
method limitations and independent comparisons are local/evidence/filter-agent.

Keep all local artifacts on D:, retry storage access after5seconds. Android
toolchains/emulators run only on GitHub Actions. Do not change the live synth.
