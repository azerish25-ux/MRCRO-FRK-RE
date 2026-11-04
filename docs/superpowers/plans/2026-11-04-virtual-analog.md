# VAnalog Reconstruction Implementation Plan

**Goal:** Reconstruct the identified VAnalog mono engine output against original
Init/Render numerical traces, without substituting a generic oscillator.

**Architecture:** A stateful portable C++ engine with fixed 32-frame processing.
Its controls remain raw recovered engine parameters. Wrapper routing, rate
conversion, voices, filtering and UI integration remain separate requirements.

**Tech Stack:** C++20, verified pitch helper, candidate MIT Plaits oscillator
primitives, CMake/CTest, GitHub-hosted Android NDK/emulator checks.

## Constraints
- Keep local files and evidence on D:; Android builds/emulators run remotely.
- Use the already-open synth for live sound observations.
- Keep original binary hashes, routine addresses and evidence limitations.
- Retain upstream licenses; verify candidate primitives against original outputs.
- Do not equate passing these raw numeric cases with complete instrument parity.

## Task
- [x] Recover Init 0x18032fe50 and Render 0x18032fef0 from the exact processor.
- [x] Generate independent original-instruction traces: nine scenarios, 40 blocks,
  1,280 output samples, including parameter changes across consecutive blocks.
- [ ] Write fixed-bit golden checks and confirm a zero implementation fails remotely.
- [ ] Reconstruct the engine and compare each scenario; investigate mismatches.
- [ ] Run the same tests on the GitHub-hosted Android emulator.
- [ ] Review and integrate verified implementation; retain remaining scope explicitly.

## Evidence
Processor SHA-256: bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74.
Local trace: ../local/evidence/vanalog-numeric-matrix-001.json.
The trace is bounded x64 emulation, not a live instrument audio render.
