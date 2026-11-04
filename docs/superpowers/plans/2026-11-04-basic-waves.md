# Basic Waves Raw Engine Implementation Plan

**Goal:** Reconstruct actual Basic Waves initialization, pitch/phase operations,
control smoothing, fixed-point interpolation and raw output from original traces.

**Architecture:** Share immutable mathematically generated tables. Preserve uint32
phase wrap, signed-short intermediate wrap, integer shifts, int64 accumulation,
float conversion order and source SRL/LPF state. Construct before audio processing;
render and prepare must allocate nothing. Wrapper level, host scheduling, voices,
filters, effects, presets and UI remain separate requirements.

**Evidence:** Original routines with independently verified initialized table data;
10 scenarios/1,280 outputs, pitch changes, phase wrap, dirty preservation, warm
controls and word overflow. Smoother equations additionally match 24,576 original
getter outputs. Explicit fixture initialization does not prove factory scheduling.

**Constraints:** D: artifacts, five-second D: retry, remote Android SDK/emulators,
live sound only from the existing open synth.

- [x] Recover initialized table inputs and non-silent original traces.
- [ ] Confirm a silent implementation fails remote reference checks.
- [ ] Implement original finite-input operations and investigate mismatches.
- [ ] Verify Android emulator output and review/integrate.
- [ ] Continue all instrument and Android application requirements.
