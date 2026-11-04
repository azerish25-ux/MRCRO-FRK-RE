# MRCRO-FRK-RE

Evidence-driven reconstruction of an installed Arturia MiniFreak V for Android.
The target is full sound, UI, interaction, preset and behavioral parity.
This is an early reconstruction: no complete Android instrument exists yet.
The portable pitch helper matches 661 original-instruction cases, and the raw
VAnalog component matches 1,920 sampled outputs bit for bit on a GitHub-hosted
Android x86_64 emulator. Android arm64 also builds;
arm64 execution and full-instrument equivalence remain unverified.

Reference installation reports Windows version **5.1.1.4933efd** (product 5.1).
No original source code is available. The project uses
[REA](https://github.com/morluto/rea) and Ghidra for authorized local analysis.

## Working constraints

- All local project files, downloads, analysis databases and controlled caches
  live under the installation's `android-port` directory on D:.
- Dot-source `scripts/Enter-ProjectEnvironment.ps1` before analysis commands.
- Android builds and emulator tests run on GitHub Actions, not the local PC.
- Further live sound observations use the user's already-open MiniFreak V.
- Original executable/plugin binaries, factory assets and detailed local evidence
  are not included in this public repository.
- `task_plan.md`, `findings.md` and `progress.md` record scope, evidence and work.

## Reproduce the initial inventory

Run from this checkout in PowerShell:

```powershell
. ./scripts/Enter-ProjectEnvironment.ps1
python ./tools/inventory_installation.py ../.. --exclude .. --output ../local/evidence/installation-inventory.json
```

The tool reads and hashes installation files, inspects PNG dimensions, and
inventories XML parameter declarations and enumeration lists. It excludes the
project directory, follows no child directory links, refuses evidence overwrites,
and executes no installed instrument code. Any unread files or XML parse errors
remain explicit in the report. Metadata is a starting point for investigation;
it does not establish sound or interaction equivalence.

## Verified native work

See [the portable core](core/README.md) for exact scope and attribution.
[The Android verification run](https://github.com/azerish25-ux/MRCRO-FRK-RE/actions/runs/41786305161)
cross-compiled arm64-v8a and x86_64, booted Android API 35 on GitHub Actions, and
reported `661 checked, 0 mismatches` for the x86_64 pitch helper.
This is numerical helper parity, not full synth or audio parity.

[VAnalog verification](https://github.com/azerish25-ux/MRCRO-FRK-RE/actions/runs/41789935056)
also passed 16/32-frame raw-engine traces and parameter-change sequences with
zero bit mismatches. The complete instrument, Android UI and APK are pending.

[The wrapper-stage run](https://github.com/azerish25-ux/MRCRO-FRK-RE/actions/runs/41794257456)
also verified stateful interpolation, VAnalog control flags and trigger latch,
and VAnalog output-level processing against original instructions. These are
scoped component checks; the full 1:1 port remains in progress.

[Basic Waves checks](https://github.com/azerish25-ux/MRCRO-FRK-RE/actions/runs/41805177708)
now also pass 1,280 raw output samples and every generated table-bank byte hash.
The original UI resource/skin/asset findings are summarized in
[UI reconstruction evidence](docs/ui-reconstruction.md); no Android UI or APK is
claimed complete.

[Raw model37 filter checks](https://github.com/azerish25-ux/MRCRO-FRK-RE/actions/runs/41813188815)
passed 7,680 original-instruction samples and 162 control blocks on the remote
Android emulator. Model49, oversampling, full voice integration and the Android
application are still pending. Additional recovered boundaries are documented in
[filter/envelope/voice evidence](docs/filter-envelope-routing.md).
