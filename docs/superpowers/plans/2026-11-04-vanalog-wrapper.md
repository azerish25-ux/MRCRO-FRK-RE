# VAnalog Wrapper Operations Implementation Plan

**Goal:** Reconstruct recovered control-word/latch updates and the original
engine -> interpolation -> supplied-level output sequence.

**Scope:** Exact selected numerical and state references for routines 0x1802c5570
and 0x1802c5380. Explicit fixture initialization is not proof of factory defaults,
GUI/MIDI scheduling, voice processing or full instrument output.

**Architecture:** Caller-owned control state plus an allocation-free render path
composing the verified raw engine and interpolator. Preserve exact float bits,
dirty bytes, consumed trigger signal, latch and phase progression through mute.

**Constraints:** All local artifacts on D:. All Android toolchains/emulators on
GitHub Actions. Live sound only from the existing open synth.

- [x] Retain wrapper assembly and recover exact scale bits 0x37800200.
- [x] Generate 64 original-instruction control state transitions.
- [x] Generate 1,280 outputs from the unchanged original wrapper with supplied
  initial history, changing parameters and levels, including mute/unmute.
- [ ] Confirm zero/no-op implementations fail the independent reference tests.
- [ ] Implement original operations and verify native/Android checks.
- [ ] Review, integrate, update evidence and continue the full synth/UI port.
