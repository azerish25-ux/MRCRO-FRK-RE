# MRCRO-FRK-RE

Evidence-driven reconstruction of an installed Arturia MiniFreak V for Android.
The target is full sound, UI, interaction, preset and behavioral parity.
This is an early investigation: no Android instrument or parity claim exists yet.

Reference installation reports Windows version **5.1.1.4933efd** (product 5.1).
No original source code is available. The project uses
[REA](https://github.com/morluto/rea) and Ghidra for authorized local analysis.

## Working constraints

- All local project files, downloads, analysis databases and controlled caches
  live under the installation's `android-port` directory on D:.
- Dot-source `scripts/Enter-ProjectEnvironment.ps1` before analysis commands.
- Android builds and emulator tests run on GitHub Actions, not the local PC.
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
