# VAnalog Reconstruction Implementation Plan

**Goal:** Reconstruct the identified VAnalog mono engine output against original
Init/Render numerical traces, without substituting a generic oscillator.

**Architecture:** A stateful portable C++ engine with up to 32-frame processing.
Wrapper decompilation established that Arturia requests 16 internal frames and
then upsamples to its 32-frame output block. Test both 16 and 32-frame paths.
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
- [x] Extend to 16 and 32-frame paths: 18 scenarios, 80 blocks, 1,920 samples.
- [x] Write fixed-bit golden checks and confirm a zero implementation fails remotely.
- [x] Reconstruct the engine and compare each scenario; investigate mismatches.
- [x] Run the same tests on the GitHub-hosted Android emulator.
- [x] Review and integrate verified implementation; retain remaining scope explicitly.

Verified at d697f017d1357386048c2f1a12f6040cbab438ac, Actions run 41789935056:
661 pitch cases and 1,920 raw VAnalog samples, zero bit mismatches on Android
x86_64. arm64 builds passed; arm64 execution and complete instrument parity are
not established. Reviewed and integrated on main without changing the golden bits.

## Evidence
Processor SHA-256: bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74.
Local trace: ../local/evidence/vanalog-numeric-matrix-001.json.
The trace is bounded x64 emulation, not a live instrument audio render.
