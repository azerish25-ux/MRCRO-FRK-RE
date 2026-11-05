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

- Corrected NotePathcaller example/header/docs using originalMiniProcess2f3760:
  sequence/control first, countconsumer onlyifnonzero, finish-control boundary
  BEFORE postseqfanout/MNFvoiceaudio. Priorwording conflatedMNFSequencer/MNFEngine.
 20byteallocatorrows,24byteMIDI-formattedpackets,20bytepostseqevents staydistinct.
- RegressiontestRED exit4 oldordering; fixedhelperGREEN3NotePath/all50Release,
  independentlysource-reviewed/compiled. Exact873f38e run41938151985passes50Linux
  andAndroidx86_64allrefs/actor-flow, bothABIscompile. Actor numeric/statecode and
  originalgoldunchanged. Originalprivatehashes retainedashistorical; newowner
  schedule/sourcebytes/currentfilehashes areexplicitinnotepath_provenance.json.
- Fulloriginalnativeaudio graphreference now640external16frame blocks/8supported
  profiles, finite/nonzero finalchannels, noDSPmodels andactualpinnedUCRTmath.
  Thisisprivateoriginaloracleevidence, NOT integratedCppparity orfactorydefaults;
  neutralcontrols suppliedexplicitly throughactualsetters. Commongraphportnext.

## Full finite source sine verified (2026-11-05 UTC)

- Shared UCRT FMA sine now covers all finite binary32 inputs. Existing ordinary math and gold remain unchanged. Root freshly replayed 662,365 broad and 11,951 compact original returns with zero differences; central production header and independent source review agree.
- New compact fixture and finite/NaN/infinity admission checks reproduce the old range failure. Three central negative controls fail 3782/3782/4 returns as expected. All 51 host and Linux Release CTests pass; exact 12dd701 remote run 41941565476 passes both Android ABI builds and API35 x86_64 reference execution. Android host sine differs on 120 full-range inputs, while the recovered helper matches every case.
- Karplus production adaptation is private and passes its original 8440-block corpus locally. Slot reuse evidence proves persistent per-voice/side models and shared slot buffers; Harmo external delay ownership and the common graph still need integration. No sounding standalone app or complete preset/UI/audio parity claim.

## KarplusStr source core verified (2026-11-05 UTC)

- Production type3 KarplusStr now has private tables/state and out-of-line implementation, persistent caller-memory/shared-RNG contracts and source unsigned counter conversion. All original expected fixtures remain unchanged.
- Exact code revision94026e7/run41942970468 passes all57 Linux Release tests, both Android ABI builds and all six Karplus checks on API35 x86_64: 8440 main blocks, 854 lifecycle commands, 720 counter-boundary blocks, 6139 import returns, seven rollback checks and 2048 allocation-test blocks. All reported differences and allocations are zero. Independent review compared23 numerical method bodies and compiled allsix production tests; root rejected33 malformed fixtures.
- Harmo shared-slot borrowing remains private: proposed typed fixed span preserves the existing59264-sample owned-buffer corpus, but direct original K/H alternating-memory gold is still pending. Exact fixed-base pitch powf candidate independently replays137161 SystemUCRT cases with zero powf/Hz differences; integration remains required. The complete native voice/effects graph, original scene rendering/touch, preset callback transport and standalone Android audio/MIDI are still active work.

## Shared native slot memory verified (2026-11-05 UTC)

- Harmo now supports the source owner-pool constructor-only profile (derived gain2.56), fixed borrowed storage, source reset and interface preparation while preserving the existing isolated owned API. Karplus and Harmo persist across selections on one caller-owned16KiB slot buffer.
- Exact4018a37/run41945301517 passes all58 Linux Release tests, both Android ABI builds and API35 x86_64 shared-slot execution:206 continuous operations/88 render blocks with zero state/memory/RNG/owner/raw/audio differences. OldH59264samples/36lifecycle commands remain exact. Independent strict source/reencoding review approved; four semantic mutants catch owned-delay, eagerInit, missingreset and clear-on-selection errors.
- Root fixed a replay-boundary error: native selection dirties OUTER slot flags, with rawmodel flags supplied later by explicit setters. Complete slot pitch/control/forwarding and full voice graph remain required. Source-derived slot-pitch and UI scene-prefix packets are now frozen for next integration, and eight preset stages are under independent review. No sounding standalone app or full UI/preset/audio parity claim.

## Native UI scene producers verified (2026-11-05 UTC)

- Compiled original expression/alias/property/initial-geometry producers now compose reviewed runtime geometry and Flex. Preserve source decimal accumulation, condition-specific alias/NaN decisions, integer sibling macros and parent-specific caches; no frozen positions or platform layout substitutes.
- Exact1135d0c/run41947451146 passes all70 Linux Release tests, both Android ABI builds and all11 scene corpora on API35 x86_64:7582 original cases0 differences/23 guards rejected. Root fresh15body comparisons,77 altered identities (11CRC collisions) and23 semantic mutants pass; independent strict actual-repo review approved.
- Full structural scene lowering, factories/event ownership, localization, paint/fonts/raster/touch remain active. Original parser proof now distinguishes compound editorlexicon framing, preserves duplicateattrs/rawcolon/fragments and literal attribute whitespace. Native slot pitch/powf out-of-line adaptation passes1160 sequential state/update/result cases and137161 original math cases privately; integration next.

## Native slot pitch verified (2026-11-05 UTC)

- Source PitchComputer/slot numeric preparation and exact fixed-base UCRT powf are now compiled behind a raw upstream field-ABI contract. Preserve source pairwise sums, nine-record dirty/cache order, ten quantizers and native dispatch flag. Private metadata/data linkage and all seven numerical definitions are verified.
- Exact03a7e7f/run41949268317 passes all72 Linux Release tests, both Android ABI builds and API35 x86_64:1160 sequential state/update/returned-value cases and137161 powf/Hz cases all0 differences/admission failures. Root fresh original UCRT/Decimal64coefficient checks,22 malformed rejections and returned-value mutant verified; independent strict actual-repo review approved.
- Common-voice integration remains next: actual upstream glide/base-note writers, model-word forwarding/dispatch and mixer/startup callback ordering. Original graph investigation found first public process flushes Osc2_Volume via2be84d→2b39f0(id37,0), so existing neutral startup profiles have zero second mixer gain despite nonzero core2 output. Preserve these histories; add post-flush/both-lane-active cases rather than claim both-slot audible coverage. Structural UI styles/with/includes and full preset listener/async behavior remain active source work. No complete standalone app claim.

## Master audio output verified (2026-11-05 UTC)

- Source16-frame48k stereo limiter/master processing is now integrated with persistent borrowed rings, exact mono/clip/smoothing order and distinct rawTiny-current versus owner-ready reset factories. Normalized pregain/postgain control conversion remains a separate join.
- Exactc8c3e7b/run41955832141 passes all75 Linux Release tests, both Android ABI builds and API35 x86_64 reference execution:288 limiter/160 preparedMaster/112 coldheld/reset/alias blocks all recorded fields and samples0differences. Seven numerical bodies exact; independently regenerated original projection matches; three lifecycle mutants detect4/5056/50 state/resource differences; independent strict actual-repo review approved.
- Next source work closes slot/voice model dispatch, parameter-memory recall and initialization ordering, complete structural UI lowering and all1210 preset compositions. The verified XML/style data-front-end packet is ready for root integration; full factories/rendering/touch and standalone audio/MIDI remain required.

## Native XML/style front end verified (2026-11-05 UTC)

- Mode0 native XML and source element-style data registration now extend mfrk_ui. Preserve raw byte/whitespace/duplicate/entity behavior and inherited clone/order/input-consumption rules; no host XML normalization or generic placeholder widgets. Native pointer/lifetime aliases remain outside this immutable data contract.
- Exactf7ab26c/run41958413328 passes all78 Linux Release tests, both Android ABI builds and API35 x86_64:650XML/323style0 differences,14guards+typedjoin pass. Five source definitions/tables exact;14 altered identities (2CRC collisions) and11 semantic mutants reject; independent strict actual-repo review approved.
- Next structural join retains raw ordered attrs and separate numeric/string contexts; do not globally expand all attrs or restore a whole context after with. Native whole-with237programs/483commands matches privateCPP, source-valid259-string context exposes earlier256 adaptercap (private4096admission extension retained for upcomingjoin). Includes/template/factory/paint/touch and scoped/zero-size audits remain open.
- Original and portable staged preset composition now coverall1210archives (954nonemptycontrolmaps+256empty),15,454,120nativewrites with0 reporteddifferences; privatepacket independentreview/productionadaptation and actuallisteners/async/DSPstillrequired. Complete voice/control/mixer and standalone app remain active work.

## Genuine Bass cores to common-voice audio verified (2026-11-05 UTC)

- Compiled private interpolation/mixing/FM routing now renders through persistent model hooks. Genuine Bass11/12 test constructs actualportedcores once, replays20sourceoperations and compares22representedmodel fields/owner flags, generated secondinput/core returns/callbackorder and mixed/FM over96continuousblocks, without runtimegoldstate/stream playback.
- Exact00750c2/run41965248844 passes all81LinuxRelease tests, both Android ABI builds and API35x86 runtimeall3suites0;2048blocks0alloc/guards pass. Sixbodymaps exact, sevensemanticmutants and11malformed controls reject, bothoriginalfixturesregeneratedidentically, independentactualrepo reviewapproved.
- Next persistent-bank join requires source-specific phase-onlyBasictrigger/runtimepolicy, VAengine-onlyInit preservingupsampler and same-slot borrowed scratch. Source-owned protocol/alias packets are nowready. Completecontroller/gates/filters/rate/FX/master scheduling/fullUI/preset loading/standaloneAudioMidi remain open.

## Scoped native UI traversal integrated (2026-11-05)

- Integrated the recovered `with` and conditional traversal into `mfrk_ui`, retaining separate numeric and string contexts and the original terminal factory boundary. The live scoped context now joins the existing property evaluator; source-valid large string contexts are admitted.
- Focused Windows Release checks passed for the 297 original scope programs, scope guards, existing string corpus, and prefix guards. Android cross-build and emulator checks are configured in CI but have not yet run for this change.
- Loops, includes, templates, factory creation, rendering and touch remain open. This layer does not make the Android instrument complete.

## Numeric UI loop profile (in progress)

- Added a bounded single numeric range to scoped traversal, including the source flag controlling numeric aliases and the one-based index. Original-instruction probes found that an exclusive range leaves its index at the next value, including for an empty range; an inclusive range removes it. The implementation retains this distinction.
- A two-range zipped numeric profile is also supported; the original processes both ranges in lockstep. A bounded nonnumeric `item_list` profile now accepts ordered names from a caller-supplied parameter provider, preserving comma splitting and one-based indices. Numeric lists, alphabetic ranges, indexed zipped ranges, the actual parameter provider join, includes, templates and factory ownership remain unfinished. Local probe evidence is retained under `local/evidence/native-ui-source/scene-lowering` on D: and the wider Android verification has not yet run.
- The real side-panel dual-button structure was exercised through the original loop and nested `with` path using the installed Sustain Polarity labels. Both flag settings produce the same button labels and inversion values in the portable scope test; actual widget creation and rendering remain open.
- Added a bounded supplied-template traversal and syntax-2 string/double declaration registration. Original argument-application probes establish sequential defaults, last duplicate argument selection, string/numeric namespaces and integer conversion for the exercised profile. A composed template/parameter-list/factory traversal reaches both side-panel button values. Predefined-style arguments, color types, colliding contexts, full native template registration and factory/render/touch lifetimes remain open.
- A D:-only integration probe parsed the installed side-panel XML (SHA-256 `fa0acf170c8f8222ca3981607e30bde4c38f083b6ad6ca8e5340f267f3fffc9b`) and internal parameter XML (`e45e75108910f3b6c9fbc290c0420f01d52efffe06b56b0610d74459477149fe`). All five side-panel templates registered; the real Sustain Polarity definition traversed to the two expected button values. This is a data-path check, not pixel or touch parity.
- The three-knob Android comparison now uses the previously recovered native center bounds, outline-gradient endpoints and marker endpoints through JNI for its explicit 125-unit integer geometry profile. Ring path, gradient raster, physical-size selection, touch and full interface remain open; remote build and screenshot checks are pending for this revision.

## Preset discrete migration integration (in progress)

- Integrated the supplied-descriptor discrete version converter and eligible numeric write selector into the strict-FP core. The installed wrapper identity is SHA-256 `354c26c58e4c8c671b50f88d070452b47c9cd6b6505b1ea18391724929b3d19e`; the original-instruction evidence and private fixture are retained on D: in `local/evidence/preset-library-agent/migrations`.
- The actual integrated source matched 10,440 discrete cases, 518 additional 129-entry cases and 800 raw numeric write cases on the local Windows Release build. Synthetic edge cases are included in the repository and configured for Android CI. These helpers do not load preset archives, construct descriptors, run instrument-specific migrations or apply values to the audio graph.

## Persistent oscillator bank integration (in progress)

- Integrated the source-derived persistent bank for Basic Waves, SuperWave, Harmo, Karplus, Virtual Analog, Waveshaper and Bass. Model selection preserves the recovered per-model lifecycle and the slot's shared scratch; this is not yet the complete voice or instrument scheduler.
- The integrated Windows Release bank matched the retained original 7,738-operation, 2,004-render sequence for every compared audio sample, random state, owner flag and final shared-memory word. The reference fixture is SHA-256 checked before use and the same check is configured for Android CI. Other models, upstream controls, filters, effects and full instrument behavior remain open.
- Added the source selection order for a previously initialized Basic slot and empty manager metadata: activate the target, reset/deactivate the old model, bind the same resource, forward phase, initialize, and mark the outer control bytes dirty. An already-active selection is a no-op. This bounded actor does not yet represent cold placeholder selection or the whole voice scheduler.
- Recovered pitch update `2da0a0`, including shared versus split glide, strict endpoint comparisons, source float/double ordering and the two slot-note writes per voice. The integrated raw-field component matched 48 original owner-flow snapshots plus 48 supplied-state original executions on Windows Release. The hash-bound pointer-free fixture is configured for Android CI; higher-level note/control writers remain open.
- Added bounded glide setup `2da290`, preserving the native prior-note selection, early snap, rate choice and mixed float/double calculation. It matches 256 original supplied-state executions on Windows Release. The fixture is hash-bound and configured for Android; the higher-level note/control writers and complete voice scheduler remain open.
- Traced all 128 MIDI note values through original public Mini processing to the packed pitch write at `2dd10d` and target setup `2d4f70`. The 0..128 descriptor's encoding and decoding now feed the recovered glide setup, with all 128 original word/target cases matching locally. Voice assignment, trigger and other note-row effects are still separate.
- Joined native-order slot control forwarding and pitch preparation to the persistent Basic Waves bank. A continuous original cold-owner sequence matched 64 blocks, including 2,048 scaled audio samples and the represented pitch/dirty state, on Windows Release. Other models, full controller setup and the downstream voice graph still require integration.
- Added a bounded note gate dispatch plan for the original two-lane poly/para and cyclic-selection paths. It matches 144 original dispatcher boundary cases, including main-envelope release/hold behavior. The owner still must perform cyclic phase resets and the other pitch/LFO/voice effects before this becomes a full dispatcher.

## Android MIDI input boundary (in progress)

- Added an allocation-free MIDI 1 byte-stream decoder with per-port state, running status, split messages, interleaved real-time bytes and streamed SysEx bytes. It is a transport boundary only: Android device discovery, timestamp scheduling, MIDI 2 UMP and routing into the native note/controller graph remain to be connected.
- Added a separate Android MIDI input library that discovers visible MIDI 1 devices, opens available output ports and delivers copied, timestamped bytes and port-reset notifications on one worker. The library is not yet joined to the decoder or app; Bluetooth discovery, MIDI 2 and physical-device verification remain open. Its remote build is configured separately.
- Added a fixed-capacity single-producer/single-consumer handoff with one decoder per port. Overflow is latched so the future audio consumer can discard queued events and reset notes/controllers rather than silently leaving notes held.
- The Android device listener now feeds that native decoder and queue through JNI. The future audio host must detach before bridge destruction and consume port-reset/overflow events. The note/controller and audio graphs remain unconnected; remote JNI cross-build and device behavior are pending.
- Added a bounded note-on/note-off adapter from decoded MIDI 1 events to the recovered NotePath actor, with an explicit channel/omni policy and synchronous publication handoff. This does not yet implement CC, pitch bend, MPE, transport, timestamp scheduling or the downstream voice owner.
- Recovered the immediate NotePath publication sink `2f36b0`: it copies each row to the owner and appends a separate MIDI-formatted record when the 256-entry queue has room. The original leaves a stack-derived byte and packet tail unspecified; the portable contract preserves them explicitly. Forty-eight original supplied-state cases match locally, including queue saturation. Downstream voice processing remains open.
- Connected decoded note routing through NotePath into the portable publication sink in a bounded integration check, applying every batch before the next NotePath operation. The app's audio/voice owner is not yet connected to that sink.

## Android audio output boundary (in progress)

- Added a fixed-block adapter and Android AAudio output boundary for the recovered 48 kHz stereo profile. It requires a real graph callback and handles variable callback sizes without allocating on the audio thread. The stream owner must handle disconnect/reopen and lifecycle off the callback thread.
- This boundary is not yet connected to the synth graph, MIDI queue or a standalone Activity. Audio focus, automatic recovery, other sample rates and physical-device behavior are not established.
