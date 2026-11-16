# Exact Windows shared GUI assets

`resources/windows-shared-assets.json` records all 2,919 files in the official MiniFreak V 5.1.1.6945 Windows package's `commonappdata/Arturia/Shared/images` member. It pins the complete installer by length and SHA256. Its processor DLL matches the existing source processor SHA256 `bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74`; the synth binaries were not replaced.

The inventory preserves filenames/case, raw byte counts and hashes, PNG dimensions and declared density directory values. Image encoding follows file signatures where available: four files have PNG payloads with a different filename suffix. The staging tool preserves those bytes, names and alpha data; it does not decode, recolor, resize or convert images. The bundled image-generation Python script is verified with the source inventory but excluded from runtime output.

`tools/acquire_windows_shared_assets.py` verifies every source record and stages 2,918 runtime files (109,656,470 bytes). It either reuses an extracted root or obtains the pinned installer and invokes `innoextract` on that one member. The installer is never executed. Existing different output files are preserved and rejected, and Windows cache/output paths must resolve physically to D:. Raw vendor payloads stay outside source git.

```powershell
. ./scripts/Enter-ProjectEnvironment.ps1
& ../local/tools/reference-python/python.exe tools/acquire_windows_shared_assets.py `
  --verified-root ../local/evidence/reference-gui-bindings-20261116/official-runtime/commonappdata/Arturia/Shared/images `
  --output ../local/build/windows-shared-gui-staged
```

For a fresh acquisition, pass `--cache`, `--innoextract` and `--output`; `--installer` optionally reuses the matching package already on D:. On a transient D: I/O error, wait five seconds and inspect/retry the same operation. Hash/schema failures require correcting their cause rather than replacing the expected identity. Android SDK builds and emulator runs remain restricted to GitHub Actions.

Full-source verification and byte-exact runtime staging passed locally. This is asset identity/acquisition evidence, not a full scene renderer, source-pixel comparison, GUI/controller binding, sound comparison or standalone Android application. The existing comparison project continues to stage only the three assets it actually consumes; this larger shared set is available for full interface integration without bloating that small comparison APK.

The official package is [MiniFreak V 5.1.1.6945 for Windows](https://dl.arturia.net/products/minifreak-v/soft/MiniFreak_V__5_1_1_6945.exe). The extractor's [upstream documentation](https://constexpr.org/innoextract/) describes extraction without executing the installer. The installed reference still needs a successful normal editor launch before live widget metadata or original-pixel parity can be claimed.
