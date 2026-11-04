# Evidence ledger

## Observations
- Initial main commit: 43df4a270f6818b10e887a160dafdb04b921a2cc; README-only repository.
- GitHub connector confirms public repository and admin/push permissions for the
  connected azerish25-ux account. No proprietary installation files will be published.
- Standalone executable: MiniFreak V.exe, 9,688,416 bytes; Windows version metadata
  FileVersion 5.1.1.4933efd, ProductVersion 5.1.
- Native plugin: MiniFreak V/x64/MiniFreak V.vst3, 36,955,480 bytes.
- Shell inventory excluding android-port: 5,795 PNG, 112 XML, 512 .mnfx, 58 SVG,
  42 GLSL, 26 TTF, 6 OTF, 82 .raw12b and 79 .raw files. These are filename counts,
  not evidence that every file is used by this version.
- resources/minifreak_vst_params.xml declares parameters, labels, defaults,
  mapping ranges and versioned oscillator type lists.
- resources/MiniFreak V_gui.xml declares UI constants and includes other XML files.
- REA source cloned from the user-specified morluto/rea repository under
  android-port/local/tools/rea. package.json declares rea-agents 6.8.0.

## Inferences
- Resource XML should materially reduce UI/parameter reconstruction work.
- The larger VST3 is a strong candidate for the main instrument implementation;
  native analysis must establish the actual architecture.

## Verified initial inventory
- Read-only inventory hashed 7,450 files totaling 336,273,072 bytes, excluding
  android-port. Evidence: local/evidence/installation-inventory-v2.json.
- Standalone SHA-256: 965ee441540945aeb1d1335c5477a8f59e00f52ce1c0b947e9caef8ce5e8c103.
- VST3 SHA-256: 354c26c58e4c8c671b50f88d070452b47c9cd6b6505b1ea18391724929b3d19e.
- 111/112 XML resources parsed. Thirty-three required an explicitly recorded
  synthetic wrapper for vendor fragments or undeclared template prefixes.
  One template (widgetEditorTemplatesGenerated/softbars/lexicon.xml) has a
  duplicate attribute and remains unparsed rather than silently altered.
- 16,283 param declarations include 13,317 in Reference_ParamNames.xml,
  2,373 in minifreak_internal_params.xml and 148 in minifreak_vst_params.xml.
  These declarations are not a deduplicated list of user controls.
- 106 item_list declarations recovered; runtime selection/version rules unknown.
- A sampled factory preset starts with Boost text archive marker
  `22 serialization::archive 10`. Full format and compatibility remain unproven.

## REA evidence
- Desktop inventory parent: ev_988f87f5d09773cf4ec6388386edbd53a00f6e7ec5eca497a5acc7ae8e126816.
  Scope: nested MiniFreak V directory, 4,129 entries; metadata/read-range identity
  only, not the full installation's content hash. No runtime candidate detected
  by this bounded classifier; that does not establish an absent native engine.
- VST3 native exports: ev_4ab41db126d4c66b3f1896f4389f2970814f120b11653af9c28b5e40ae4dd63c.
  PE amd64, 324 exports. GetPluginFactory at 0x18057bca0;
  InitDll at 0x18062b230; ExitDll at 0x18062b240.
  Export names alone do not establish the synth's DSP implementation.
- REA 6.8.0 provider doctor passed Ghidra 12.1.4, portable Node 24.22.1,
  Temurin JDK 21.0.13 and Windows native isolation controls on NTFS D:.
  All acquired tools and retained evidence are under android-port/local.
- Native VST3 Ghidra import launched; no completed engine analysis yet.

## Unknowns
- DSP implementation, exact algorithm/parameter semantics, preset encoding and
  runtime behavior have not been established.
- No Android app or audio parity has been implemented or verified.
