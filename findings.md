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

## Processor and live application
- Actual processor: MiniFreak V/resources/x64/minifreakvProcessor.dll;
  6,151,920 bytes; SHA-256
  bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74.
  It was already included in the hashed inventory. An earlier missing-DLL
  concern was an analyst path-selection mistake, subsequently corrected.
- C:/ProgramData/Arturia/MiniFreak V is a symbolic link back to the D: resource
  tree. Read-only module inspection of the existing standalone process confirmed
  it loaded this processor and the selected VST3. The user confirms live sound.
- Processor export evidence ev_0f21760f3c6e55724999644ebde8538d34360d669b128e4f50e381aac4eef844:
  GetProcessorFactory at 0x1802a8990. Decompiled factory embeds 5.1.1.6945.
- Completed processor overview ev_fdbfa2a82a8a5fa00eaed2e470a985e7cc419ec34d392dca6e8b027aff5ff98d:
  Ghidra identified 4,579 local/external procedures and 3,731 defined strings.
  This is not a proof that every function/string was discovered.
- Full names evidence ev_ef00744b531773e564ab0a694e15f3114ccd43e07715b7013b82bb08d6bcfdba
  exceeded MCP delivery size; complete data is in
  local/evidence/processor-evidence-001.bundle.json, not lost or truncated.
- RTTI/vtable labels identify Plaits engines, Arturia MiniFreakVCF,
  MiniFreakDualVCF, MiniFreakFX, MNFEngine, resampling and voice allocation classes.
  Labels are architecture seeds; runtime paths need separate validation.
- VirtualAnalogEngine vtable at 0x180540cf0 has slots 0x18032fe50,
  0x1802a3390, 0x18032fef0 and 0x1803303d0. Inspection indicates Init,
  no-op Reset, Render and an additional state operation respectively.
  Initial filenames containing guessed method names are not semantic evidence.
- The Render candidate contains the characteristic variant-2 Plaits formulas.
  Its object layout/output paths differ from current upstream; copying upstream
  unchanged is not a verified port.
- Pitch helper 0x18032e950 clamps note-9 to [-128,127], uses two lookup
  tables, and multiplies by approximately 0.00028645832f. Upstream Plaits
  includes a hardware clock correction; installed constants must be preserved.

## UI and sound observations
- Resource declaration graph: 109 parsed resource XML files, 29,860 nodes,
  one explicitly unparsed malformed template. Full expressions/parent-child
  relationships retained locally; includes and conditionals are unevaluated.
- Existing live app screenshot shows Home/Advanced/Sequencer views and preset
  8 Saw Key. One virtual-keyboard click was performed, with no parameter changes.
- Live speaker-loopback capture local/evidence/live-synth-002: 25 seconds,
  stereo 48 kHz, finite samples, peak 0.0924062728881836. Source was the already-open
  synth, with system-output mixing and GUI timing limitations explicitly recorded.
- Before the user's live-only steering, offline warmup capture reference-warmup-002
  yielded nonzero audio (peak 0.11183501780033112); repeated renders differed.
  Neither capture is a deterministic full-instrument equivalence fixture.
- 21 pitch-helper vectors were generated by bounded Unicorn 2.1.4 emulation of
  the original instructions/tables. This is static emulation, not live instrument
  audio or proof of Android equivalence.

## First reconstructed component
- Expanded original-instruction reference set: 661 cases, including notes 0..127
  with selected fractional boundaries plus clamp/out-of-range cases. Local source:
  local/evidence/pitch-helper-vectors-v2.json. Public fixed bit pairs are in core/tests.
- Complete comparisons of the 256 used entries in each upstream MIT pitch table
  matched installed float32 bytes. Evidence: local/evidence/pitch-tables-comparison.json.
  Upstream stmlib commit e3bd7c9cc00e4364166f9905c0509b6ffd0535ec.
- Portable C++ pitch conversion preserves the installed constant, table truncation,
  clamp and operation order. Header explicitly requires finite inputs.
- TDD red run 41785655927 built successfully and rejected all 661 cases against
  the temporary zero implementation; green native checks then passed.
- Verified run 41786305161 at commit fcc41cf83f548af83cce5a99517c3497b2236a8f:
  native checks passed; Android arm64-v8a and x86_64 cross-compiles passed;
  API 35 x86_64 emulator reported `661 checked, 0 mismatches`.
  arm64 execution, oscillator audio, complete instrument behavior and UI are not
  established by these checks. The reviewed component was fast-forwarded to main.
- VAnalog original routines Init 0x18032fe50 and Render 0x18032fef0 were emulated
  for four 32-frame blocks at one parameter setting. Local numeric trace:
  local/evidence/vanalog-numeric-001.json. This is a reconstruction seed, not a
  portable oscillator implementation or a live sound capture.
- Saved live Home, Advanced and Sequencer reference images on D: under
  local/evidence/live-synth-002. They are tool-provided JPEGs, useful for layout
  reference but insufficient for lossless pixel-equivalence claims.
- Observed Advanced and Sequencer navigation changes the window's view/height.
  Returned to Home. Original patch controls were not modified.

## Raw VAnalog reconstruction
- Reviewed component integrated on main at d697f017d1357386048c2f1a12f6040cbab438ac.
- Original-instruction trace cases: 18 scenarios, 80 blocks, 1,920 output samples
  across 16 and 32-frame paths, including parameter changes across consecutive calls.
- Actions run 41789935056 passed native checks, Android arm64/x86_64 builds and
  Android API 35 x86_64 execution. Logs report 661 pitch cases and 1,920 VAnalog
  samples with zero bit mismatches. Output-range guards also passed.
- Exact matching required installed variable-saw scaling (`sample * 1.6666666f -
  0.8333333f`) and the original variable-shape wrap-correction multiplication order.
  All expected golden bits remained independent of the replacement code.
- Correcting a flattened pseudocode grouping by itself was unsuccessful; assembly
  evidence ev_0340a7d5b86c4999322dc598828ea4b53cb8c9ac3aaa713e22bae3ed8178289e
  resolved actual operation order. Do not assume decompiler text preserves every
  floating-point grouping.
- Pinned MIT primitive sources and modified hashes are in third_party/manifest.json.
  Render performs no allocation or I/O. These sampled cases do not establish all
  inputs, arm64 execution, full patch output or complete instrument behavior.

## Wrapper evidence and next component
- FreakVirtualAnalog<32,96000> has vtables at 0x180544d78 and 0x180545f18.
- Wrapper render at 0x1802c54b0 places 0x10 in the raw Render size argument, then
  calls 0x1802c9e60. Assembly evidence:
  ev_79e8a731d1f7322d72726ca2edb4d567eaf5428e2d0a9d6e4d63650ef982370d.
- 0x1802c9e60 is a stateful linear 2x interpolator. For each of 16 inputs it writes
  (previous + current) * 0.5, then current, updating previous across blocks.
  Pseudocode evidence ev_dc0bf6d8b951b3f983784b5294447d60ce0b042f9b85bf96d1efb0180a994bda.
  It is not yet ported or independently verified with numeric vectors.
- Method 0x1802c5570 converts dirty ushort controls into engine parameters at
  offsets +0xe0 (harmonics), +0xd8 (timbre), +0xdc (morph), using an approximately
  1.525972e-05 factor. Exact factor bits, upstream UI/control mapping and full
  object initialization remain to be established. Method 0x1802c5310 sets note
  storage at +0xd4. Method 0x1802c5380 applies the float level at +0x28.
- Do not infer a default gain from unrelated initializer offsets; multiple
  inheritance and this-pointer adjustments need explicit validation.
- Fresh hashes of standalone, VST3 and processor still match initial identities.

## Verified interpolation and VAnalog wrapper operations
- Stateful interpolation integrated at 5b1da34a638671db3d3b59cfef83d930490643b9.
  Original-reference zero implementation failed in run 41791767015; run
  41792381316 passed 1,920 direct and 1,280 composed VAnalog-chain samples with
  zero mismatches on Android x86_64, plus existing checks and both ABI builds.
- Control scale is exactly 0x37800200 at 0x180555eac, float value
  1.5259720385074615e-05. Read evidence:
  ev_5c60bbed9ba90e2864881f35b6989e4010f627e14deae69f0fb1a747aad20211.
- 64 independent state-update references cover all dirty masks, changed and
  preserved controls, words through 65535, rising/held/released trigger states
  and noncanonical nonzero byte flags. Dirty flags and words are not cleared
  by the recovered routine. Note is preserved by this operation.
- Output-stage references execute the unchanged native virtual wrapper, raw
  engine, interpolation and supplied-level multiplication with explicit fixture
  initialization. Nonzero initial history and muted-then-unmuted blocks are covered.
- Run 41793812538 rejected no-op controls (64 mismatches) and zero output
  (1,195/1,280 mismatches), while previous tests passed.
- Run 41794257456 at d2340082ff1857d4a8f1dc515202fc4533cd2dae passed 64 state
  updates and 1,280 output-wrapper samples, zero mismatches on Android API 35
  x86_64. Existing checks and Android arm64/x86_64 builds passed. Reviewed and
  integrated on main. arm64 execution, full factory initialization and GUI/MIDI
  scheduling are still unverified.
- Method 0x1802c17c0 sets the input signal byte +0x30. Method 0x1802c5320 writes
  data at +0x108/+0x110; it is not proven to be an output-level setter.

## Basic Waves investigation seeds (not ported)
- Primary vtable 0x1805449e0; initializer 0x1802cf840, control update 0x1802cf8c0,
  raw render 0x1802cfb30, pitch/table selection 0x1802c6870, phase seed setter
  0x1802c6850. Source files are in local/evidence/session-dsp-002.
- Raw render uses three uint32 phase accumulators, 512-entry signed-short tables,
  16-bit interpolation fractions, signed-short interpolation deltas, signed
  coefficients and float output scaling. This differs from the floating Plaits
  VAnalog implementation and must not be replaced with a generic oscillator.
- Pitch setter converts supplied frequency with approximately 44739.242 and
  22369.621 factors into phase increments, and chooses table-bank pointers from
  0x18046b310 using a band derived from its third argument. Exact constants and
  caller semantics still need validation before implementation.
- The initial table at 0x1805b0b10 and the sampled bank-17 table at 0x1805b1b10
  are zero in the unloaded image. Constructor-only silence is not valid audio
  equivalence evidence. Runtime table initialization must be reconstructed.
- xrefs lead to table initializer 0x180275fa0: it builds a 512-element signed-short
  sine table using sin(), amplitude 32767*0.8, +0.5 and integer truncation.
  Other bank initializers and actual populated table bytes remain unresolved.
- Aggregate initializer 0x1802e6fb0 constructs multiple generators. The filename
  basic-waves-constructor.json is an investigation seed, not proof it constructs
  only Basic Waves. Its Basic Waves base helper is 0x1802ea3f0; pointer initializer
  0x1802ea790 initially assigns 0x1805b0b10.
- The aggregate writes 0x3ee6e979 at Basic Waves instance level offset +0x28 and
  0x3f8ccccd at both VAnalog instance level offsets +0x28. These are observed
  initialization writes, not proof of final preset/GUI output levels.

## Resolved Basic Waves table initialization and verified raw engine
- Independent scoped reads of the already-open matched processor captured 128
  bank pointers and 34 unique 512-short tables. All pointers matched the original
  static array after rebasing; every table was read twice with equal bytes.
  Private identity/ranges/hashes: local/evidence/wave-table-agent/live-tables.json.
- Original initializer instruction emulation with explicitly modeled sin/memset
  imports matched all 17,408 unique shorts. Independent scalar formulas also
  matched. The import model was checked against live bytes rather than assumed.
- Default sine uses sin(2*i*0.006135923151542565), then *32767, *0.8, +0.5 and
  truncation. Fourier variants sum ascending harmonics, then *26213.600000000002,
  +0.5 and truncation. Negative rounding is intentionally asymmetric.
- Saw coefficients are reciprocal n times signed 2/pi; square uses 4/(pi*n) for
  odd n; triangle uses reciprocal n*n times alternating 8/pi^2 for odd n.
  Exact constants, harmonic caps, routing and provenance are in the agent's
  FINDINGS.md, bank-formula-map.json and portable-formula-verification.json.
- Portable production code mathematically generates tables; no live table snapshots
  are shipped. Hash fixtures preserve independent original expectations.
- Red run 41800434464 rejected zero-filled tables in all 128 banks. Green run
  41800863961 at b36de66e68dcd1d3b79722e49d6fe73781144d38 matched all 128 byte
  hashes after Android x86_64 generation, with earlier checks and both ABI builds.
- Original Basic Waves method traces were then generated with populated verified
  table inputs. They execute original constructor, pitch/phase, reset, control
  prepare and raw-render instructions, with explicit fixture initialization.
- Red run 41804549537 rejected the silent implementation on 1,279/1,280 samples.
  Green run 41805177708 at e71bd2f3315965bb75bcf85a043e8bbef3c73210 matched all
  1,280 samples on Android API35 x86_64. Cases include frequency changes, phase
  wrap, dirty preservation, warm controls, word overflow and reverse frequency.
  All earlier checks and table hashes passed. Reviewed and integrated on main.
- Power-of-two probes of original setters confirmed scale bits 0x472ec33e for
  main increment, 0x46aec33e for sub increment, and 0x4f800000 for phase seed.
  Evidence: local/evidence/basic-wave-constant-probes.json.
- Shared SRL/LPF finite-input equations matched 24,576 original getter outputs.
  Preserve SRL ctor step0x3bda740e: recomputing defaults gives0x3bda740d.
  LPF coefficient0x3d064870 and relative threshold semantics are preserved. Reset
  sets target/current without recomputing threshold. Settled means exact equality;
  do not snap stagnant values by introducing an epsilon comparison.
- Limits: sampled raw processing and mathematical tables, not complete voices,
  factory scheduling, all inputs, presets, actual Android app or arm64 execution.

## Expanded original UI evidence
- Local report/maps: local/evidence/ui-resource-agent. Seventeen source documents,
  22,580 expanded declarations, 699 hashed 100% drawable/font assets, both skins.
  MACRO_NB=4 resolves selected loops; conditions/actions remain declarative.
- All five ASTR wheel containers decoded to 640 original PNG byte slices. CRCs,
  zlib payload sizes, dimensions and output hashes passed without re-encoding.
- Visible synth knobs use procedural gradients/styles, not the legacy numbered
  image knob families. Native rendering and gesture behavior still need work.
- Declared skin differences include keyboard ranges and counterintuitive bindings:
  mf37 uses KeyboardNote49, MIDI48..84; mf49 uses KeyboardNote37, MIDI36..84.
  GUI-loaded expressions write MiniFreakDualVCF_Mode from the skin constant.
- Source-based implementation constraints are summarized in docs/ui-reconstruction.md.
  No completed Android renderer, live mf49 screenshot or lossless pixel equivalence
  is established by these maps or bounded JPEG baselines.

## Model37 filter and envelope/voice boundaries
- Raw model37 initialization, normalized control maps, double smoothing,
  nonlinear feedback, LP/BP/HP, reset, noise and DC blocker are reconstructed.
  Original preferred addresses and limits: docs/filter-envelope-routing.md.
- Independent initialization matched 72 fields; normalized prepare/core matched
  5,184 outputs and 162 control blocks without copied initialized coefficients.
- Added 1,536 impulse/reset samples and 960 noise samples with five seeds and
  on/off/on toggles. Noise disabled preserves seed; reset does not reseed.
- Red run 41812198195 rejected silence/no-op with 5,337 mismatches; other tests
  passed. Green 472182b148c3a498a6cf4616a3ac8c588e5a2e1f/run41813188815 passed
  all 7,680 VCF samples/162 blocks plus earlier checks on Android API35 x86_64.
  arm64-v8a also compiled, not executed. Host eight tests passed.
- Independent review found no required fixes and verified 120 constants and
  both table byte sequences against original extraction. Separate first-only
  and force-only transition vectors remain a coverage limit.
- Main/cyclic envelope preparation, gates/steps and shared descriptor mappings
  are recovered in local/evidence/envelope-agent. Portable envelopes remain open.
- Actual Voice constructor2e5cd0/stride1a000, prepare2cd760, envelope step2d7fa0,
  render2d81b0 and containing engine2be9b0 are mapped in local session-routing-001.
  Display indices0..5/6..11 select oscillator1/2 within the same six voices;
  this is not evidence for a second six-voice bank or twelve-voice polyphony.
- Model49, dual/oversampling, complete routing/allocator, other models, effects,
  sequencer, presets, Android UI/audio/MIDI and full-instrument parity remain open.

## Fixed model49 verification and next filter boundary
- Reconstructed audio-rate modulation and four original calibration curves in
  Vcf49. Model49 uses current normalized cutoff/resonance regardless dirty;
  mode/noise use dirty-or-force only. Its double caches are unused by render.
- Shared nonlinear/DC/noise/reset extraction preserves all model37 references.
- Red run41814746059 rejected silence on8,907 comparisons (8,775samples plus
  seed checks); all earlier tests passed. Green2c04d2f861a28285fd544488a8757b6c7d80ea06,
  run41815081333 passed all9 tests and Android API35 x86_64 output comparisons,
  including8,775model49 samples/135blocks and7,680model37 samples/162blocks.
  Both Android ABIs compiled; arm64 execution remains unverified.
- Independent review approved fixed49/shared stage; every calibration-table
  bit matches original extraction. Integrated on main; no full filter claim.
- Exact8-section IIR factor2/4/6/8 equations independently match9,600up and
  1,920down original values with60state-block pairs; wrapper async/fades pending.
- Exact six-filter owner isModuleCloner constructor2e05b0, children+70/stride2920,
  count6, constructor wrapper2e45d0, callerxref2d6572. Check PE pdata for owners;
  earlier proximity guesses2e44e0/2deaf0 were wrong and are not graph evidence.
- Main ADSR scalar handoff now independently matches3,600curve bytes,224gates,
  5,760steps,20preparations,512dirty masks and4resets. Preserve signed32 wrap
  before decay-target widening and adjacent-field table endpoints. Details in
  local/evidence/envelope-agent/PORTABLE_ADSR.md. Portable C++ remains next work.

## Verified filter IIR and settled chain
- Integrated3a5b28aec7e9ddb2c82fdb88ec5120c020fdb4ed after independent review.
  Exact original2/4/6/8 factors,8sections, separate up/down states, insertion
  gains, first-phase decimation and reset. Startup-only wrapper buffer allocation.
- Red41816094920 rejected placeholder resampler/chain with15,336 combined
  mismatches; all earlier checks passed. Green41816365296 passed all10 native
  tests, both Android ABI builds, x86_64 API35 execution and prior checks.
- Android exact comparisons:9,600up values,1,920down,60paired state blocks,
  1,920 actual original settled-wrapper outputs/PRNG states. Chain host rate96k.
- Fade/resource lifecycle and full dual/filter graph still open; raw37/raw49
  and settled49 are not proof of complete voice/instrument/application parity.
- ADSR C++ candidate local/envelope-agent/cpp-candidate matches original traces
  and tables, but requires constructor curve prewarm before repo integration.
  Public all-nine-controls admission contract and test-only fixture loading
  must remain explicit. No Android ADSR validation yet.

## Main ADSR integrated and verified
- Integratedb80b5bfef4ea1eb07d6be44df97de7043528630a after arithmetic/provenance
  review and startup prewarm fix. Production has unconditional test-access friend,
  fixture loader only in tests, no mutation branches. Construct off callback.
- Red41817831985 rejected3,336table bytes/6,624state records; other checks passed.
  Green41817910133 passed all11 native tests, both Android ABI builds and x86_64
  API35 execution:8,032commands/7,032asserted records/3,600curve bytes, zero errors.
- Original FX01 fixture2,283,246bytes compacted losslessly to242,791bytes FX02;
  every33cell array survives, including unasserted cells/op6loads. SHA256
  600ab02f2e8117d4a7db992ac0f4103dc5a99aa408d2a489d389e19ed4f9214d.
  Root generators and independent review regenerate and decode byte-identically.
- Caller/clock/envelope-copy roles, public dispatch and cyclic envelope remain open.
- Filter transition evidence now runs actual numeric resource construction within
  original worker/completion schedule (bounded ownership/import shell), exact audio
  and states. Unready leaves output untouched and PRNG frozen. Not portable yet.
- Adaptive factor3 uses original zero-gain/one-section fallback, factor1 directcopy;
  do not substitute invented3x coefficients. Separate FIR/resource paths remain.
- UI HOME runtime scene recovery started from source defaults/conditions/styles;
  no actual Android renderer/app/APK or complete UI parity yet.
