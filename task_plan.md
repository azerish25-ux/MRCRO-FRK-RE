# MiniFreak V Android parity project

## Objective
Reconstruct the installed MiniFreak V as an Android app with verified 1:1 sound,
UI, UX and functionality. No original source code is available. Completion is
unproven until all feature and equivalence requirements are independently verified.

## Constraints
- Repository: https://github.com/azerish25-ux/MRCRO-FRK-RE
- Installation: D:/MAIN SOFTWARE/Synths/MFRK/main synth/MiniFreak V
- Project: installation/android-port; checkout: android-port/repo.
- All project-controlled downloads, temporary files, caches and analysis databases
  must remain under android-port on D:. Never change the installed binaries.
- If D: returns a storage-access error, wait five seconds and retry the affected
  operation before diagnosing a persistent blocker. Preserve evidence and check
  whether a write completed before repeating a mutation.
- Android builds and emulators run on GitHub Actions, never this local PC.
- Further live sound observations use the already-open MiniFreak V, per the
  user's updated instruction. Do not instantiate another synth for sound captures.
- Use morluto/rea for artifact analysis. Keep evidence identity and limitations.
- Commit and push project work. Keep proprietary installed binaries/assets and
  sensitive local evidence out of the public repository.

## Phases
1. COMPLETE: establish repository, D: environment, artifact identities, REA.
2. IN PROGRESS: inventory parameters, UI definitions, resources, preset formats.
3. IN PROGRESS: map native engine and construct deterministic reference capture host.
4. IN PROGRESS: reconstruct and verify each DSP subsystem against reference renders.
5. PENDING: reproduce preset/state and complete instrument behavior.
6. PENDING: implement faithful Android UI, interactions, audio and MIDI.
7. IN PROGRESS: remote Android builds/emulator tests; full-app parity audit pending.

## Acceptance evidence
- Exact version and hashes for every reference executable/resource set.
- Exhaustive parameter/control/feature matrix with observed behavior and tests.
- Reference/port audio comparisons across engines, modulation, voices, effects,
  sample rates, presets and performance events. Deterministic and stochastic
  comparisons distinguished; no generic oscillator substituted for recovered DSP.
- Screenshots and interaction comparisons for every view, menu and state.
- Preset compatibility and state restoration proven with representative and edge cases.
- APK builds and emulator functionality checked on Actions; real-device latency,
  thermals and touch performance remain unverified without actual device evidence.

## Errors encountered
- Local gh CLI has no authenticated account. Connected GitHub account has confirmed
  admin/push access; use connector for writes where native Git auth is unavailable.
- Host Node 24.18.0 is below REA 6.8.0 supported runtime; acquire a portable supported
  runtime under D: before executing REA. RESOLVED: verified portable Node 24.22.1.
- Standard XML parsing missed vendor fragments/undeclared template prefixes.
  RESOLVED with documented in-memory wrapper; three regression checks pass.
- One vendor template contains a duplicate attribute; retained as an explicit
  inventory issue. No installed resource was modified.
- PowerShell ConvertFrom-Json needs -AsHashtable for the inventory's empty
  extension key (extensionless factory presets). RESOLVED in analysis commands.
- Initial wrapper cold import exceeded 330,000 ms. Provider cleanup completed;
  retained its failure report and raised startup budget to 1,800,000 ms in a
  persistent local client. Separate processor import completed successfully.
- Full list_names exceeded the MCP transport budget, but analysis was retained.
  RESOLVED: exported complete evidence bundle and read the retained symbol array.
- First offline capture was silent. Warmup plus avoiding an immediate state
  restore produced audio; root cause is narrowed to initialization/state timing,
  not fully isolated. Per updated user instructions, further sound observations
  use the already-open synth.
- SoundCard 0.4.5 called NumPy binary fromstring removed in NumPy 2.5.4.
  RESOLVED: pin local reference environment to NumPy 1.26.4; live loopback succeeded.
- Embedded Python excludes the script directory from sys.path. RESOLVED for the
  emulation helper by inserting its explicit local tools directory.
- Initial Android CI had no sdkmanager on PATH. RESOLVED with pinned remote
  Java/SDK setup. Its v3 default requested the removed 'tools' package; explicitly
  selecting platform-tools fixed that. NDK and emulator tests now pass remotely.
- A stale sky screenshot ID rejected one view-navigation attempt before input.
  RESOLVED by re-observing the same app before retrying. Final view restored Home.
- VAnalog candidate initially had 304/1,280 float-bit mismatches. Preserving the
  installed saw output scaling reduced this to two. A naive-accumulation grouping
  hypothesis increased mismatches to four and was reverted after assembly review.
  Original wrap-correction multiplication order resolved the remaining two.
- A PowerShell foreach pipeline metadata query required wrapping its results in
  @(...); corrected before execution of the intended read-only checks.
- A late REA client read ended with exit1 before a reply; its cause is unconfirmed.
  After five seconds, D: reads/hash checks succeeded. Complete exports remain;
  start a fresh owned CLI client only when further provider queries are needed.

## Next Step
Integrate SuperWave and verified voice gain/pan/Q14 stages, connect ADSR and the
deferred filter into the original voice/allocator/Dual graph, then
integrate Android audio/UI from the recovered scene specification. Continue every
remaining engine, effect, sequencer and preset/state requirement. The verified
Basic Waves/VAnalog components are not completion of the instrument.
