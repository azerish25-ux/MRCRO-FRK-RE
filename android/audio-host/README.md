# Android audio output foundation

`AudioOutput` opens a low-latency AAudio output stream for the currently
verified 48 kHz, float-stereo profile. It adapts variable callback sizes to
16-frame engine blocks without allocating in the callback. A real graph render
function is required; failure stops the callback. The owner serializes stream
lifecycle calls and handles reported disconnects on a non-audio thread.

This is not yet connected to the voice/effects graph, MIDI queue, lifecycle of a
standalone Activity, audio focus, automatic restart or other sample rates. A
successful cross-build does not establish real-device latency or sound parity.
