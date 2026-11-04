# VAnalog Upsampler Implementation Plan

**Goal:** Reconstruct the original 16-to-32-frame interpolation and verify its
composition with the recovered raw VAnalog engine.

**Architecture:** A stateful, allocation-free C++ module with fixed input/output
extents and caller-supplied initial history. Exact operation order follows
processor routine 0x1802c9e60. This does not establish wrapper gain or control routing.

**Constraints:** Local files stay on D:. Android builds and emulator execution
run only on GitHub Actions. Live audio observations use the already-open synth.
Keep numerical emulation distinct from live audio and full instrument parity.

- [x] Revalidate the repository and live REA processor session.
- [x] Retain original interpolation and wrapper-call evidence.
- [ ] Generate independent direct and VAnalog-chain references.
- [ ] Verify a zero implementation fails in remote native CI.
- [ ] Implement the recovered operation and verify all existing and new checks.
- [ ] Run Android emulator checks, review, and integrate verified work.
- [ ] Continue full wrapper state/control mapping and remaining synth/UI subsystems.
