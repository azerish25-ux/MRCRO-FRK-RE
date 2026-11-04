# Pitch Helper Implementation Plan

> For agentic workers: execute this plan task by task using the approved sequential workflow.

**Goal:** Reconstruct and verify the original finite-input pitch conversion helper
as the first portable engine component, while preserving the full Android goal.

**Architecture:** A pure C++ float function with verified matching lookup tables.
Tests consume original-instruction reference vectors, not expected values computed
by the replacement implementation. Host tests and Android executable checks run
remotely, with no local Android installation.

**Tech Stack:** C++20, CMake/CTest, GitHub Actions, Android NDK and Android emulator.

## Global Constraints
- All local project-controlled storage stays under android-port on D:.
- Android builds and emulator tests run only on GitHub Actions.
- Preserve exact source artifact identity and distinguish emulation from live audio.
- A passing helper test does not mean the instrument is ported.

## Task 1: Verified pitch helper
- [x] Recover the original helper's pseudocode and file identity.
- [x] Generate independent finite-input vectors from original x64 instructions.
- [ ] Verify upstream pitch-table bytes match the installed table bytes.
- [ ] Create core/include/mfrk/pitch.hpp, core/src/pitch.cpp and attribution.
- [ ] First write core/tests/pitch_test.cpp using fixed input/output bit pairs.
- [ ] Confirm the missing implementation fails in remote native CI.
- [ ] Implement exact table/clamp/float operations and confirm native CI passes.
- [ ] Cross-compile the test for Android and execute it on an Actions emulator.
- [ ] Review results, update the parity ledger and publish the verified change.

## Subsequent work
Recover parameter routing, oscillator implementations, wrappers, modulation,
filters, effects, voices and sequencer behavior; verify each against the reference.
Integrate preset/state compatibility, Android audio/MIDI and all original UI states.
Complete the exhaustive parity audit before claiming the full goal is achieved.
