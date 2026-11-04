# Android filter renderer comparison foundation

This is an actual Java/Android project for the source-defined lower three knobs of the mf37 HOME filter panel. It is a test harness and reusable Canvas widget foundation, not a complete instrument or a claim of native pixel parity. No local Android SDK, Gradle distribution, emulator, installer or vendor application was downloaded/executed for this work. The asset-free project is tracked here; generated vendor assets and build outputs remain ignored.

## What is concrete

- RendererComparisonActivity has no launcher intent filter, audio engine, controls masquerading as a full instrument, or warning text painted over the comparison canvas.
- RendererComparisonView renders an explicit logical crop[1815.6,535,560,205], original background placement[0,-413,3200,1878], three125x125 source knob bounds, source labels and explicit normalized parameter assignments. The exact crop/scale/assignments/source IDs are exported with screenshots.
- SilverSoftKnob is a reusable native Canvas primitive driven by original baseknob/silverknob inheritance: source colors, gradient endpoints, center/ring/outline/indicator/shadow attributes and exact local fonts. It uses no Material widget, legacy knob PNG or unrelated redesign.
- app/src/main/assets/filter-slice.json is asset-free derived metadata, retaining source IDs/style chains/zones. Actual font/background bytes are never checked into source.
- tools/acquire_assets.py and acquisition-manifest.json reproduce715 hash-verified original resources directly from official Arturia; tools/stage_renderer_assets.py stages only the two required fonts and original synth_BG into app/build/generated/vendor/assets. Gradle rejects missing/wrong generated asset hashes. Instrumentation separately checks packaged font hashes.

## Source exactness versus unverified raster choices

The original XML constants, styles, bindings, fonts/assets and derived rectangles are preserved. Native flex rounding, shader equations, angle units, gradient stops, center-radius interpretation, indicator radial mapping, shadow kernel, label baseline, font antialiasing and colorspace have not been recovered. They are explicit candidate conventions in RasterConvention/SilverSoftKnob and comprehensively listed in provenance/raster-assumptions.json. In particular angle2.4 is currently interpreted as a half-sweep in radians; source ring gradient color is used in a candidate radial gradient, not a claimed recovered native shader.

Only the comparison's0/.5/1 endpoint/center vectors map directly to indicator positions; the general magnetic-zone inverse, snapping, hover/value-format lifecycle and gestures remain unimplemented. Center text is empty in these non-hover captures; callers may supply explicit test strings. No feedback buffer, synthetic filter curve or fake DSP data is drawn. This prevents a source-based prototype from being mistaken for completed behavior.

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

Only asset-free source checks ran: Python AST/XML parsing, Java delimiter/package checks, source geometry/style/default consistency, selected hash membership, no binary assets under src, and no launcher entry. tools/check_candidate.py passes; provenance/static-checks.json records the precise limits. Remote runs41853761969 and41857992066 built both APKs and passed lint. The first failed repeat stability; the second reused one Activity/View and retained screenshots, revealing the Android immersive-mode education overlay. Renderer assertions have not yet passed. The runner now acknowledges that first-use tutorial before launch and records focus in each capture. The workflow system-image revision is recorded in sdk-packages.txt; pinning API35 alone is not a guarantee of cross-revision pixel identity.

Next proof is an unobscured API35 screenshot run with all distinct/repeat assertions passing, followed by controlled original-raster calibration. The broader original HOME/Advanced/Sequencer UI, gesture dispatch and full instrument remain active future work outside this deliberately small comparison foundation.
