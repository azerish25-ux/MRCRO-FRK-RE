# Native engine reconstruction design

This records the architecture and parity approach the user approved before
authorizing implementation. It does not replace the full Android objective.

The instrument engine will be portable C++, separated from parameter/state
handling, Android audio/MIDI and the faithful UI. Components are reconstructed
from the exact installed processor and checked against original evidence before
being integrated. No generic synth is substituted for the reference instrument.

The first component is the finite-input pitch conversion helper at
0x18032e950 in processor SHA-256
bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74.
It maps a float note to the engine's normalized frequency; naming does not imply
a public host sample-rate contract. Preserve clamp bounds, table indices,
float32 rounding and multiplication order. Preserve upstream MIT attribution
where verified matching upstream tables are reused.

Validation uses reference vectors from bounded emulation of original instructions,
including clamp boundaries and fractional-note quantization. Native and Android
executables must match those bits. All Android toolchains and emulator execution
run on GitHub Actions. This component's success does not establish oscillator,
filter, resampling, modulation, whole-instrument audio or UI parity.

Future engine tasks require their own evidence and integration checks. UI
reconstruction uses the observed resource declarations, original appearance,
runtime state comparisons and explicit touch equivalents. Live sound observations
use the user's already-open synth per the updated instruction.
