# Android filter renderer comparison foundation

This is an actual Java/Android project for the source-defined lower three knobs of the mf37 HOME filter panel. It is a test harness and reusable Canvas widget foundation, not a complete instrument or a claim of native pixel parity. No local Android SDK, Gradle distribution, emulator, installer or vendor application was downloaded/executed for this work. The asset-free project is tracked here; generated vendor assets and build outputs remain ignored.

## What is concrete

- RendererComparisonActivity has no launcher intent filter, audio engine, controls masquerading as a full instrument, or warning text painted over the comparison canvas.
- RendererComparisonView renders an explicit logical crop[1815.6,535,560,205], original background placement[0,-413,3200,1878], three125x125 source knob bounds, source labels and explicit normalized parameter assignments. The exact crop/scale/assignments/source IDs are exported with screenshots.
- SilverSoftKnob is a reusable native Canvas primitive driven by original baseknob/silverknob inheritance: source colors, gradient endpoints, center/ring/outline/indicator/shadow attributes and exact local fonts. It uses no Material widget, legacy knob PNG or unrelated redesign.
- The 125-by-125 comparison profile now calls the recovered C++ center-bounds, outline-gradient endpoint and marker endpoint calculations through JNI. These use the original integer-dimension and packed-trigonometry behavior; Android Canvas still performs the actual drawing.
- app/src/main/assets/filter-slice.json is asset-free derived metadata, retaining source IDs/style chains/zones. Actual font/background bytes are never checked into source.
- tools/acquire_assets.py and acquisition-manifest.json reproduce715 hash-verified original resources directly from official Arturia; tools/stage_renderer_assets.py stages only the two required fonts and original synth_BG into app/build/generated/vendor/assets. Gradle rejects missing/wrong generated asset hashes. Instrumentation separately checks packaged font hashes.

## Source exactness versus unverified raster choices

The original XML constants, styles, bindings, fonts/assets and derived rectangles are preserved. Center bounds, outline-gradient endpoints and marker endpoints use the recovered native calculations for the explicit integer-size profile. Native physical-size selection, flex rounding, ring path, shader interpolation, stroke raster, shadow kernel, label baseline, font antialiasing and colorspace remain unresolved. They are listed in provenance/raster-assumptions.json. The ring arc and radial gradient remain candidates, not recovered native shader behavior.

The captured comparison scenarios use explicit0/.5/1 indicator assignments. Native magnetic phase/delta math is now available through the bridge described below; full getter/paint-input binding, pointer sensitivity, hover/value-format lifecycle and gesture dispatch remain unfinished. Center text is empty in these non-hover captures; callers may supply explicit test strings. No feedback buffer, synthetic filter curve or fake DSP data is drawn. This prevents a source-based prototype from being mistaken for completed behavior.

## Verified compatible build pins

- AGP8.7.3, Gradle8.9, Java17; compile/targetSDK35, minSDK26.
- Android's official AGP8.7 notes explicitly support API35, require Gradle8.9/JDK17, and list8.7.3. Build-tools35.0.0 is explicitly installed on the runner (AGP's documented minimum/default is34.0.0).
- Gradle8.9 official binary checksum: d725d707bfabd4dfdc958c624003b3c80accc03f7037b5122c4b1d0ef15cecab.
- No local Gradle wrapper binary is needed; the Actions job downloads the pinned distribution remotely and verifies its SHA256 before use.

Primary sources checked:
https://developer.android.com/build/releases/agp-8-7-0-release-notes
https://docs.gradle.org/current/userguide/compatibility.html
https://services.gradle.org/distributions/gradle-8.9-bin.zip.sha256
https://developer.android.com/reference/android/app/Instrumentation
https://developer.android.com/reference/android/app/UiAutomation

## Integration and remote proof

The project lives at android/renderer-comparison; .github/workflows/android-renderer-comparison.yml runs for scoped pushes/pull requests and manual dispatch. It pins checkout/setup-java/setup-android/emulator actions and upload-artifact by commit. All SDK/Gradle/APK/emulator work occurs on GitHub Actions. No secret/account is required for official asset acquisition.

The workflow acquires/verifies original assets outside source git, stages three generated inputs, builds app+instrumentation APKs and lint, installs both on API35 x86_64, and runs the native Instrumentation harness. It exports full-view and exact-crop PNGs, parameter/source/transform metadata, font identity checks, SDK/Gradle versions and logs. Artifacts contain requested APKs/screenshots/reports; raw package caches and loose fonts are not artifact inputs.

The five frames are minimum[0,0,0], center[.5,.5,.5], declared-factory subset[1,0,.5], maximum[1,1,1], and factory-repeat. The harness checks exact assignments, nonempty native drawing, crop bounds, changed frames and repeated-frame pixel stability. It does NOT label these images as native-reference goldens. The saved original SawKey JPEG has different state/scale and is not silently used as a lossless oracle.

## Validation performed locally

Only asset-free source checks ran: Python AST/XML parsing, Java delimiter/package checks, source geometry/style/default consistency, selected hash membership, no binary assets under src, and no launcher entry. tools/check_candidate.py passes; provenance/static-checks.json records the precise limits. Remote runs41853761969 and41857992066 built both APKs and passed lint. The first failed repeat stability; the second reused one Activity/View and retained screenshots, revealing the Android immersive-mode education overlay. Run41860166281 passes all five onscreen captures after acknowledging that tutorial and waiting for actual draw events. All captures retain focus; four states have distinct crop hashes, and factory-repeat exactly matches factory-default. provenance/remote-validation.json records the report and APK/artifact identities. The workflow system-image revision is recorded in sdk-packages.txt; pinning API35 alone is not a guarantee of cross-revision pixel identity.

Next proof is controlled original-raster calibration and recovery of native widget drawing/gesture equations. The passing comparison-harness checks do not establish native pixel parity. The broader original HOME/Advanced/Sequencer UI, gesture dispatch and full instrument remain active future work outside this deliberately small comparison foundation.

## Native magnetic control bridge (in progress)

The three controls now bind their existing literal zone declarations to reusable magnetic control state. `controlPhase` accepts the recovered raw phase integers; `mapControlDelta` returns the source view-space result and skip flag through JNI without changing a parameter. Source `f59680` state transitions and `f59930` mapping include wrapped periods, parameter mismatch, plateaus, point zones and skipped zones. Fresh Windows checks match 5,040 phase and 6,674 delta cases; the older 648-case nonwrapping profile remains covered.

The native bridge returns packed bits, retaining signed zeros. JNI rejects malformed/out-of-profile zones and invalid wrap periods. Remote instrumentation checks exact native results, the actual envelope control's declared zones and phase state. No Android build ran locally. These raw phases do not imply Android MotionEvent actions. Pointer sensitivity, clamping, reverse writeback, actual parameter dispatch, tooltip/hover lifecycle and full touch interaction remain unfinished. Raster assumptions above remain unchanged.

The bridge also exposes the source magnetic wheel feature. It retains separate `handled` and `hasTarget` flags: a consumed event at the outer boundary can have no write target. Reversed-wheel direction, raw-value reversal, inertial/zero filtering and nearest adjacent zone jumps match 3,744 original cases locally. This feature alone does not implement default wheel handling or Android wheel/touch adaptation.

## Android touch capture and controller attachment

`SourceTouchRouter` captures stable Android pointer IDs independently for different controls, retains ownership outside the hit rectangle, processes historical movement before the current sample and releases each edit once on up/cancel. A second finger cannot take an already captured control. Coordinate conversion retains source `a67f10`: binary64 addition of 6755399441055744 followed by low-word extraction, including nearest-even half-pixel ties. Android keyboard/button state is translated to JUCE modifier bits separately from pointer action routing.

`RendererComparisonView.setTouchTargets` accepts actual control targets and supplies source-local coordinate transforms through `controlCoordinates`. Binding replacement, resize, focus loss and detachment cancel captures. The default comparison view has **no controller targets installed**: its drawing assignments remain explicit. Capturing a target asks the parent to retain this touch stream; drag completion is separate from an accessibility tap.

`NativeSliderController` dispatches down/drag/up/cancel to an opaque native attachment. The application's native GUI host registers a `ControllerAttachment` that retains its actual `SliderController`, metadata, states and services. Handles are monotonically assigned IDs, never native addresses. Detach releases an active edit before destroying its owner; detach requested during native dispatch is completed after the callback. Java input and attachment disposal require the UI thread. The attachment API supplies no guessed parameter metadata or plain-assignment controller fallback.

Debug builds include `NativeGestureProbe`, an explicitly synthetic observer host used only by instrumentation. It exercises the real recovered controller, sensitivity, magnetic phase/delta and writer with literal test metadata. Instrumentation covers reordered pointer indexes, two independent fingers, cancellation, historical samples, fine modifiers, a real scaled View hit/coordinate path and active-owner disposal. These checks prove Android/core routing for that supplied profile, **not** the real widgets' controller/metadata, hover/reset timers, full UI, pixel or DSP parity. The debug source is excluded from the release CMake branch. [Run 43882334393](https://github.com/azerish25-ux/MRCRO-FRK-RE/actions/runs/43882334393) built both Android ABIs, the instrumentation APK and lint, then passed these checks and the five existing renderer captures on API35. `provenance/touch-remote-validation.json` records the exact source head, artifact digest and proof limits.

API references: [Android pointer IDs and history](https://developer.android.com/reference/android/view/MotionEvent), [JUCE modifier bit definitions](https://docs.juce.com/master/classjuce_1_1ModifierKeys.html). Source binary evidence and the recovered controller profiles remain recorded with the native core fixtures.
