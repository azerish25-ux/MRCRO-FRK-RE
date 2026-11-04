# Android parity acceptance

The delivered app must reproduce the installed MiniFreak V sound, controls,
visuals and behavior, including small details. It must launch as a standalone
Android instrument with every function reachable through touch and straightforward
MIDI connection. The current repository contains verified native components and
an Android comparison harness; the full instrument remains in development.

## Original interface and touch

Preserve original screens, layout, fonts/assets, colors, numeric formatting,
menus, disabled states, feedback, editing, precision adjustment, reset, learning,
modulation routing, preset browsing and tutorial behavior. Recover mouse-only
semantics before choosing equivalent touch gestures. Keyboard chords and separate
controls need correct multi-pointer down/move/up/cancel handling. Interface zoom
and viewport access must retain original geometry rather than redesigning it.

Live inspection has recorded Home, Advanced, matrix/macros, LFO shaper, Sequencer,
all four sidebar sections, engine/filter/FX/component-preset menus, zoom, standalone
audio/MIDI preferences and browser filtering/banks. These observations are a
partial survey. Tutorial warning was inspected and canceled; no tutorial playback
or import-restoration test has yet been performed.

Exact native layout evidence already distinguishes scaled absolute dimensions
(int(double(float scale)*logical double+.5)) from later Flex edge truncation.
At80% the three filter controls are50pixels square; at100% they are63pixels.
Geometry, packed trig, gradient/marker math and magnetic adjustment have native
instruction comparisons. Complete scene binding, paint/raster/font and gesture
lifecycle still require implementation and validation.

## Included presets and dependencies

Preserve626 browser factory presets,72 tutorial states and512 included hardware
backup slots, with their distinct original roles. The256 empty hardware Init
archives retain their original meaning. Preserve names, bank/designer/type/style
metadata, covers, declarations, dependency references and all161 installed sample/
wavetable assets. The current acquisition candidate reproduces1380 runtime files
from pinned official payloads. Producer hardening and CI integration are pending.

Component FX choices are also included:63 choices across11 families, repeated
for three slots. Their source-defined aliases route through saved FXn_Opt1;
actual DSP modes and related rate/synchronization handling still need closure.
Versioned oscillator/effect/volume/input-gain enums require the original migration
rules. Copying a normalized legacy value into a current enum can select another
sound. Preserve counted binary data embedded inside Boost text archives.

## Standalone MIDI and audio

Use the Android MIDI device model for device/port enumeration and lifecycle.
The [official MidiManager API](https://developer.android.com/reference/android/media/midi/MidiManager)
defines USB/Bluetooth/virtual byte-stream transport and a separate Universal MIDI
Packet transport onAPI33+. Implement and verify the applicable transport paths,
port direction, timestamping, device removal/reconnection and message framing.
Compatibility claims must follow evidence for actual device/transport classes.

Reproduce original note/velocity, sustain, bend, pressure, controllers, channel/
MPE, clock and source-specific protocol behavior. The recovered NotePath opaque
per-note key must retain its native meaning until the host mapping is proved.
The actor consumes canonical packets, not an external MIDI byte stream. The full
voice/FX/host graph and Android audio/input integration remain required.

Android SDK/build/emulator work runs on GitHub Actions. Physical-device behavior,
latency and controller compatibility remain separate validation evidence.
