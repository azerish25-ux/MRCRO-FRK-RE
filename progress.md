# Progress

## 2026-11-04
- Started authorized inspection. Kept installation unchanged.
- Created android-port/local/{tmp,cache,tools,evidence} on D:.
- Cloned reference analysis tooling and project repository to D:.
- Established version metadata, initial resource counts and GitHub access.
- Loaded reverse-engineer-anything and planning-with-files skills for this work.
- No Android SDK, emulator or build toolchain installed locally.
- Full 1:1 objective remains active and incomplete.
- Installed checksum-verified portable Node/JDK/Ghidra and pinned REA 6.8.0 on D:.
- REA desktop inventory, export table and standalone PE-resource reports retained.
- Inventory completed for 7,450 files. Recovered vendor XML fragments without
  modifying source files; one genuinely malformed template remains flagged.
- Three focused XML-reader regression checks passed locally.
- Added GitHub Actions workflow for analysis-tool checks. Android emulator
  workflow will follow the actual Android build target; no app placeholder
  is being described as a completed instrument.
- Published a36e61c143a5e91c2de4e4e7ecac536baae40d30 to main. Remote Actions run
  41782948010 completed successfully (three resource parser checks).
- Retained complete processor overview, procedures, names and focused decompilations.
- Captured live sound from the already-open synth as requested; no patch controls changed.
- Generated original-instruction pitch-helper vectors; full app port remains incomplete.
- Published extended analysis at 601ef79f7eaba9510dd178a97266c7bdf774aa9a.
- Reconstructed pitch helper through a remote red/green cycle. Run 41786305161
  passed native checks, Android arm64/x86_64 builds and 661 bit comparisons on
  a GitHub-hosted Android API 35 emulator. Reviewed and integrated on main at
  fcc41cf83f548af83cce5a99517c3497b2236a8f. No local Android SDK/emulator was used.
- Generated a four-block numerical trace of the original VAnalog routines in
  emulation. The portable VAnalog engine has not yet been implemented.
- Saved live Home/Advanced/Sequencer JPEG baselines on D: and restored Home view.
- Reconstructed raw VAnalog and resolved float operation-order differences without
  relaxing bit comparisons. Integrated reviewed d697f017d1357386048c2f1a12f6040cbab438ac.
- Run 41789935056 passed 661 pitch checks plus 1,920 original-instruction VAnalog
  output comparisons in a GitHub-hosted Android API 35 emulator. arm64 and x86_64
  Android cross-compiles passed; arm64 runtime remains unverified.
- Recovered the actual 16-frame raw engine -> 32-frame 2x interpolation wrapper.
  Next work is the interpolator and wrapper controls/state, followed by the other
  instrument subsystems and all Android UI/functionality. No APK exists yet.
- Continued: original-instruction upsampler tests rejected the zero implementation,
  then passed 1,920 direct and 1,280 composed VAnalog-chain output comparisons.
  Run 41792381316 also passed existing checks and remote Android ABI builds/emulator.
  Reviewed and integrated at 5b1da34a638671db3d3b59cfef83d930490643b9.
- Generated 64 original control-state transitions and 1,280 outputs by executing
  the unchanged native VAnalog wrapper with explicit fixture initialization.
  Current wrapper implementation/tests are in the red phase on a feature branch.
- Old processor helper session 2309 exited with code 1 during a read request;
  reason unconfirmed. Complete earlier exports remain intact. Started an owned
  replacement CLI client and successfully reimported the same processor.
- User added a D: recovery preference: wait five seconds and retry storage-access
  errors before treating them as persistent. No storage relocation to C: is allowed.
- Reconstructed VAnalog control state and output composition. Run 41794257456
  passed 64 original state updates and 1,280 output-wrapper samples on the remote
  Android emulator, with all earlier checks green. Reviewed and integrated
  d2340082ff1857d4a8f1dc515202fc4533cd2dae.
- Traced Basic Waves control, rendering, pitch/table selection and initializers.
  Its image tables are initially zero; table-generation recovery is required
  before meaningful Basic Waves numeric waveform references can be established.
- Continued: independent waveform-table investigation recovered all 34 populated
  mathematical tables. Scoped live reads, original-instruction initialization and
  scalar formulas agreed byte-for-byte for 17,408 unique shorts. Sources/evidence
  stay under ../local/evidence/wave-table-agent. No live synth was changed.
- Portable table generation passed all 128 bank byte-hash checks in Android run
  41800863961; integrated b36de66e68dcd1d3b79722e49d6fe73781144d38.
- Raw Basic Waves implementation passed 1,280 original-instruction outputs and
  all previous checks in Android run 41805177708; reviewed/integrated
  e71bd2f3315965bb75bcf85a043e8bbef3c73210. arm64 builds only, not execution.
- Independent smoother analysis verified 24,576 getter outputs and the original
  constructor/recompute coefficient difference. Original finite-input behavior
  was preserved rather than introducing tolerant settling or recomputed defaults.
- UI resource investigation decoded all 640 physical-wheel PNG frames, resolved
  MACRO_NB=4 and mapped 22,580 declarations/both skins. Local report/maps are in
  ../local/evidence/ui-resource-agent; source-based summary is docs/ui-reconstruction.md.
- Remaining: full voice envelopes/filters/allocator and routing, remaining models,
  FX/sequencer/preset compatibility, actual Android application/UI/audio/MIDI and
  device-level validation. No full instrument or APK completion is claimed.

## Live analysis handles (revalidate before reuse)
- Deferred controller integrated63ddbfe193e939d53b3a1df1848a3fa6f567d99a;
  Android41836390093 passes all14tests/lifecycle/cache references,0callbackallocs.
  Both ABIs compiled; x86_64 executed. Full objective stays active.
- SuperWave candidate now nativefloatphase/callerdefault0,2016fixtures, allmutants
  fail; reviewapproved. Nextrootintegration. FXinput/Q14candidate likewise ready.
- IntegrationREA58961 received normalclose byui_resources; confirmterminal before
  reuse. Wave/oscillator ownedREA closed; no roothandlelive. Full graph/UI pending.
- Numeric FIR/bypass/fallback/selector/fade/resource stage integrated d71cb685...
  Android41821648527 attempt2 passed all12 checks and new original references.
  First attempt failed old adb-pull; no code/numerical change was needed to retry.
- Filter wrapper handoff now complete locally with108indexedREA records; wave_tables
  closed ownedREA and is doing a final setter-conversion probe without newREA.
- ui_resources owns liveREA58961 under envelope-agent/integration-001/rea and is
  tracing FX inputgain/fullvoice wiring. Do not borrow this handle.
- oscillator_models owns new independent oscillator-bank/model investigation;
  vcf_review completed localHOME scene handoff and numeric resource review.
- Full deferred controller, remaining DSP models/effects/state, Android audio/MIDI
  and faithful UI/app still required. No APK or completion claim.
- Main ADSR integratedb80b5bfef4ea1eb07d6be44df97de7043528630a; Android41817910133
  passes8,032commands/7,032records/3,600bytes plus all earlier checks. Both ABIs
  compiled, x86_64 executed. Required constructor prewarm fix reviewed/resolved.
- ui_resources now traces full voice envelope/gate/gain/clock bindings; wave_tables
  investigates actual filter factory/transitions/FIR/fallback factors; vcf_review
  now recovers source-backed mf37 HOME runtime scene under ui-resource-agent/runtime-home.
  No whole-instrument or APK completion claim; objective remains active.
- Exact IIR/settled49 chain reviewed/integrated3a5b28aec7e9ddb2c82fdb88ec5120c020fdb4ed.
  Android41816365296 passed9,600up/1,920down/60pairedstates and1,920chainoutputs,
  plus all previous checks; both ABIs built, x86_64 executed. Full objective active.
- ADSR C++ candidate reviewed: arithmetic/state/table provenance passes, required
  integration fix is off-audio-thread curve prewarm. Candidate/no-op and widened
  arithmetic negative checks are retained under envelope-agent/cpp-candidate.
- Root83522 is now confirmed terminal(exit1) after explicit MCP Connection closed
  and stalled close request. Only this owned client was interrupted; exports safe.
- Fixed model49/shared nonlinear stage integrated on main at
  2c04d2f861a28285fd544488a8757b6c7d80ea06 after independent review and Android
  run41815081333:8,775model49 outputs/135blocks,7,680model37 outputs/162blocks,
  all previous checks. Both ABIs compiled, x86_64 executed, full goal active.
- Main ADSR independent mathematical curve/state model is ready for C++ port;
  scalar handoff and expanded wrap/dirty/reset evidence are in envelope-agent.
- Exact IIR resampler scalar core recovered; original complete filter wrapper
  and async/fade lifecycle are under independent investigation in filter-agent.
- Root83522 returned explicit MCP Connection closed on binary_session. Its saved
  routing responses remain. Close request sent; do not reuse it for queries.
- Raw model37 VCF passed native/Android run41813188815 at472182b148c3a498a6cf4616a3ac8c588e5a2e1f:
  7,680 samples,162 control blocks,zero bit mismatches; all earlier checks passed.
  Both Android ABIs compiled; x86_64 emulator executed. Independent review approved
  the scoped implementation with first-only/force-only coverage limits. Integrated
  on main at472182b148c3a498a6cf4616a3ac8c588e5a2e1f; full objective remains active.
- Model49/oversampling investigation continues independently in filter-agent.
- Root routing responses are retained in session-routing-001; see latest status above.
- Envelope analysis client48056 closed normally; local envelope-agent retains
  original instruction probes, shared mappings and system-CRT comparison evidence.
- Processor client exec session 78124 is now terminal: exited1 on a late read-only
  assembly request. Cause unconfirmed; after a five-second retry/check, D: reads
  and original processor hash were healthy. Do not reuse this terminal handle.
  Retained responses: ../local/evidence/session-dsp-002.
  Old session 2309 is terminal; its responses remain in ../local/evidence/session-dsp.
  Latest new-session export: ../local/evidence/processor-evidence-007.bundle.json.
  Earlier full symbol inventory: ../local/evidence/processor-evidence-003.bundle.json.
- Persistent wrapper client exec session 73850; import completed after about
  991 seconds. Overview identifies 66,875 procedures and 37,450 defined strings.
  Export: ../local/evidence/wrapper-evidence-001.bundle.json. Do not duplicate imports.
- Node REPL has sky, synthWindow and synthState bindings for the observed live app.
- Detailed evidence and acquired upstream sources are local-only under ../local.

- SuperWave/FX input/mix integrated on main05f32eac; independent review approved.
  Run41857702525 passes18 host checks and all Android API35 x86_64 references,
  both ABIs compile. Recovered UCRT sine fixes Linux247sample/36history mismatch;
  Android compact28246 returns0mismatches, hostlibm differs385. Source-gold exact
  fixtures and numerical admission are documented in core/README.md.
- First real renderer-comparison APKs/lint and715resource acquisition passed
  run41853761969; screenshot repetition failed. Reuse-one-window and diagnostics
  fixes pushed ui/renderer-comparison67f4e15f; run41857992066 pending.
  This is3knobs/test-only; full instrument APK/UI parity remains incomplete.
- Next actual graph work: NotePath candidate integration-002/cpp-notepath in
  progress; Bass candidate ready; full Harmo investigation active with closed
  table dependencies. All generated project material remains on D.

- Bass merged main5e9c4481 after independent review and run41859731514:19native
  checks/all Android x86_64 references pass,776Bassfixtures14400samples exact.
- Android comparison merged50d5b486 after run41860166281:APKs+lint andfive
  actual screenshot cases pass, allfocus true, fourdistinct andexactrepeat.
  Source-controlled project isthree-knobtestonly; full app/UI parity NOTdone.
  Artifacts/hash report retained locally and provenance/remote-validation.json.
- Exact UIUX objective reiterated. Native SoftKnob rendering/gesture RE active
  atnative-ui-source, actualsource/RTTI/function ranges found. Old wrapper
  CLI73850 died on benignsessionquery, emptyreservedrequest preserved; allits
  processesclosed. Worker ownsfreshisolatedclient; originalinstalledbinaryintact.
- Harmo fullcandidate and NotePath candidate are under independent development;
  neither is repository-integrated or Android-verified yet. Continuefullgraph.

- Harmo merged3a16b889 after source/independent review and run41865340832:21host
  andallAndroidrefs pass,59264raw+scaled samples/state/delay/tables exact,
  36lifecyclecommands exact. Privatevalue-state andexplicitzeroarena/FPcontract.
- WholeNotePath merged94c4902e after run41885314678:24host+Androidrefs pass,
  7172commands5.8Mfields all0; actor/publication/finish contracts documented.
- Userexpandedacceptance: everyincludedpreset, everyfunctiontouchusable,
  standaloneAndroid andhasslefreeMIDI. docs/android-parity-requirements.md tracks
  this alongside alloriginalUI/sound features. Fullinstrument remainsactive.
- OriginalCUGUIinspection resumed onuserreopenedPID240. OriginalpatchNativeZIP
  retainedDwithCRC/dependencyproof;2488live-sharedserializedscalarsmatchbackup.
  OneOsc2menuChordsselection wasrestoredBasicWaves; all11Osc2valuesbitmatch.
  RuntimeDsurvey recordsoriginalviews,menus,bank/type/stylepanels anddeviceUI.
- NativeUI helpers/fullFlex numeric gold independently source-proved; original
  scene/default/property binding andfullraster/font/gesture parity remainopen.
- Includedlibrarycandidate1380runtimefiles/1210presetarchives+assets/metadata
  reproducedfromofficialpayloads. Independentreviewfoundpath/atomic-temporary
  producerfixes; noCI/mainassettoolintegrationclaimyet. Versionmigrations,
  componentFXmode/rates andfullDSPremainrequired.

- Included library producer merged4078698 after independent review and successful
  Actions run41904333272/job126834838082:47 Linux focused cases, zero skips,
  all four real file-symlink regressions, all1380 original file identities and
  byte-identical repeat reports. Preserves626 browser/72 tutorial/512 hardware
  archives plus dependencies; no native preset-loading/audio parity claim.
- Exclusive native UI survey extended through every Osc1 model's control labels,
  Osc2 Chords plus six processing modes, sample/wavetable browser/import filter,
  About, macro/matrix Learn/amount/reset, expression panel and chord note editing.
  See docs/native-runtime-ui-survey.md. Osc2 visible21 retains30 native descriptor
  slots, including9Dummy entries; normalization count is not visible-menu count.
- Final native History restoration and Home80 view verified20:38:48UTC:2488
  mapped scalar values exact,134LFO/shaper and1622sequence fields covered;
  five UI prefix fields, native1280x616 root, three original filter knob bounds
  and all originally captured style values match initial. Original screenshots
  remain compressed UX observations, not lossless pixel-equivalence proof.
- Full sounding standalone Android app, complete UI/UX/touch/MIDI and every
  preset's sound remain incomplete. Reviewed WaveShaper repository adaptation,
  source scene binding and native preset migration/default paths continue.

- Waveshaper merged6fc3d79 after different-author source/API/wiring review and
  successful finalrun41910253799:29LinuxRelease checks/allAndroidx86_64 references,
  both Android ABIs compile.1612blocks/51584raw+scaled samples/2313generatedtable
  values/320lifecycle commands all exact;512definedstress and39releaseguards pass,
  zero ordinary callback allocations. Full admitted state/trace comparisons held.
- Private state/Tables API requires explicit zero-arena and Init. Source phase
  and reset are no-ops; Init preserves outer history. Active non-ok statuses roll
  back outputs/state, without claiming equivalent native fault audio. Original
  packedgold and provenance remain immutable; docs/waveshaper details scope.
- Next active source work: seven numeric UI primitives production adapter,
  KarplusStr buffers/kernel/table producers, native preset eligibility/empty
  hardware-slot and legacy WT AudioSample migrations. Full standalone Android
  sound/UI/touch/MIDI/preset parity remains active and incomplete.

- Native UI primitives merged93e7ee0 after rootoriginalinstructionreproduction,
  additional30geometry/1728magnetic sourcecases, different-author repo/wiring
  review and successful Actions41915285460:37LinuxRelease checks/allAndroid
  x86_64mathreferences,15domain/boolean guards and hashknownvectors pass.
  Seven originalcasesets total38310 observations,0mismatches; bothABIscompiled.
- mfrk_ui STATIC/PIC/IPO-off/strictFP owns seven puremath APIs,10exactbodies.
  All7sourcegoldfixtures unchanged.49alteredfiles(includingCRCcollisions) and
  7semanticmutants independentlyreject. Full native geometry/dragmath is bounded;
  calleradmissions, original scene/paint/font/input lifecycle remain required.
- WarmprocessorREA30608 nowownedwave_tables(targetbd036), avoidingduplicates;
  Karplus96blocksourcekernel matching expands. Wrapper75012 remainsownedby
  integration_review, nowpackaging complete nativeFlex andnestedlayout profiles.
  Presetwork continues nativeeligibility/emptyHWslot andAudioSamplemigration.
- Entire standaloneAndroid/UIUX/touch/MIDI/everypresetsound objective remains
  active and incomplete. See docs/ui-primitives.md and remotevalidation record.

- Native full/nested Flex mergedc279057 after source/build/workflow review,
  independent originalmachine-code regeneration and exactrevisionrun41920532205.
  Linux40ReleaseCTest pass; Android x86_64API35flat4964/35802 float+integeredge
  differences0,nested144trees720boxes/2880floatrectangles0;14guardsreject.
  BothAndroidABIscompile,arm64runtimeunverified.
- Twofullsolver bodies preserved; sourceprivateState doublecalculation/public
  floatbounds,stableties,additiveshrink,stickyline-total/freezeretry,signedmargins,
  strictwrap,floatreverseendpointandnativeownednestedrecursion retained.
  Independentextra72conflicting/signedconstraintcases216rectangles and8trees
  depths1..16/171rectangles matchactualcompiledlibrary.14altered/12mutantsreject.
  SourceComponentsetters/callbacks,scene/font/raster/inputstillseparate.
- Wrapper75012terminal(no process/no savedProgramDB); authorizedONEreplacement
  wrapper11743 ownedintegration_review, target354c/Java11748, servingpresetqueries.
  Processor30608 remainswarmownedwave_tables; no duplicatenativeimports.
- RuntimeLayoutactorcandidate delivered privately: sourceitem cacheLengthf32+20,
  geometrydoublefieldswidenfloat. Initialsource7bodies/6fixtures/20guards proven
  privately; notyetmerged or targetverified. Do notreplaceitscachewithf64.
- Karplus continues fullfiniteUCRT+sustainedcounterwrap proof; actualphaseposition
  uses zero-extendeduint32 conversion, notsignedcounter. Fullinstrumentgoalactive.

- Runtime resizev2 merged010843a after rootindependent source/ASM/corpus review
  and exactrevisionrun41930772735:50LinuxRelease checks/allAndroidx86_64refs0,
  bothABIscompile.9corpora39025observations/20236itemupdates;20guardsreject.
  All16RuntimeItemflags, combinedfixed+ratio andnonzero ratio minima retained.
- Expandedoriginalparentcasesexposed206cachemismatches: fixed+A|Brefresh before
  guardandchildsizecallbackrefresh beforecached-length accumulation are required.
  v2correctsactualsourceprotocol; parentforcescrossfill, directC0stillno-observer.
 5oldhelpersunchanged,4moved/source-provedprotocoldefinitions explicitlymapped.
 63alteredfiles/19mutants/3historicalguardREDcontrols discriminateoldnarrowlogic.
- Historicalgeometryoracleentryalignmentauditcorrected Win64RSP to8mod16;
  rootindependentlyreplayed all4016geometrycases withproperframes, rawgoldexact.
  Author'sseparate8892scene-helperaudit also0; no golden/sourceinstructionchanges.
- Nextactualsourceintegration: sceneconstruction/property/named-expression
  pipeline, fullfiniteKarplus/sharedowner and actualcommonvoice/audio graph.
  Whole app/fullUI/touch/MIDI/preset sound scope remains active and incomplete.
