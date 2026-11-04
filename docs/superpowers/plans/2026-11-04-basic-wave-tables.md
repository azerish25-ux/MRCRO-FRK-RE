# Basic Waves Mathematical Tables Implementation Plan

**Goal:** Reconstruct populated mathematical waveform tables and all 128 bank
selections with original byte equivalence before porting the Basic Waves engine.

**Architecture:** Startup-only double-precision scalar generation of 34 immutable
512-short tables. Preserve ascending harmonic summation, coefficients, scaling,
asymmetric +0.5/truncation and exact bank routing. No shipped table snapshots are
production dependencies. Tests use independent original SHA-256 references.

**Evidence:** Scoped, double-read stable snapshots from the already-open matched
processor, original-instruction initialization with modeled sin/memset imports,
and independent formulas agree for all 17,408 unique table shorts. Local evidence
is in ../local/evidence/wave-table-agent. This establishes tables, not synth audio.

**Constraints:** D: local storage, five-second retry on D: storage-access errors,
remote-only Android SDK/emulators, no new live synth instances.

- [x] Revalidate independent table and bank-map evidence.
- [x] Confirm zero-filled generated tables fail all bank checks remotely.
- [x] Implement recovered mathematical generation and verify exact byte hashes.
- [x] Run Android emulator checks and review/integrate.
- [ ] Initialize original Basic Waves numeric references and port its processing,
  then continue all remaining instrument and Android UI requirements.

Verified b36de66e68dcd1d3b79722e49d6fe73781144d38, run 41800863961:
all 128 bank selections match original byte SHA-256 values after Android x86_64
generation. Previous checks and both Android ABI builds passed.
