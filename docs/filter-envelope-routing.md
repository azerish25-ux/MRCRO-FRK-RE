# Filter, envelope and voice reconstruction

Reference: processor SHA256
`bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74`.
Addresses are preferred-image addresses, not live process addresses. Detailed
REA responses and probe scripts remain in D: local evidence directories.

## Raw model37 filter

`Vcf37` reconstructs initialization, normalized cutoff/resonance mapping,
double-precision control smoothing, reset, three output modes, nonlinear
feedback, saturation, deterministic noise and DC blocking in32-frame blocks.
It preserves the original arithmetic grouping and approximation functions.
Construction allocates once; preparation/rendering do not allocate. Caller
supplies the original first/force/dirty lifecycle and initial noise seed.

Original methods: constructor `0x180321c20`, initialization `0x1803222f0`,
reset `0x180322520`, preparation `0x1803225b0`, rendering `0x180322860`.
Controls are finite normalized0..1; sample rate must be positive.

Independent initialization matches72 original fields across48/96/192kHz.
The independent normalized-control model matches162 control blocks and5,184
outputs, without copying original initialized coefficients into its inputs.
Additional original traces cover1,536 impulse/high-resonance-reset samples
and960 noise samples across five explicit seeds. Noise is unsigned xorshift32
with shifts13/17/5; disabled noise preserves the seed. Reset does not reseed.
The tests compare float bits and double target bits without a tolerance.

Local evidence: `local/evidence/filter-agent`; fixture generation for the
additional impulse/noise traces is `tools/generate_vcf37_extra_vectors.py`.
The original exp/expf imports are explicitly modeled in offline emulation;
their fixture arguments were separately checked against the installed CRT.
This is bounded reference evidence, not exhaustive input equivalence.

The dual wrapper `0x1802feb30` selects model37 or an oversampled model49 branch.
The latter has separate bulk processing and audio-rate modulation, and is
not implemented. Its upsampler is not the oscillator's simple2x interpolator.
No completed filter placement in a full voice/instrument is claimed.

## Recovered envelopes

Main ADSR: vtable `0x180545b50`; constructor `0x1802e6b20`, reset
`0x1802ce630`, prepare `0x1802ce650`, step `0x1802e0a70`, gate
`0x1802e0e40`. Recovered states include idle, forced maximum, attack, decay,
sustain, release and a five-tick pre-release/retrigger stage. Accumulation is
Q31 with a Q15 output. Main attack, decay, sustain and release mappings have
been linked to the shared descriptor objects passed to the voice constructor.
Gate/legato behavior must retain the recovered state machine, not a generic
floating-point ADSR approximation.

Cyclic envelope step `0x1802e5a20` and shared knot evaluator `0x1802e8720`
serve the embedded CycEnv. Recovered modes include Env/Run/Loop and three
Rise/Hold/Fall orderings. Rise/fall/hold time maps use fourth powers with
different affine offsets. The FreakCycEnv wrapper also embeds a second ADSR.

Local evidence: `local/evidence/envelope-agent`, including224 gate transitions,
2,880 ADSR steps,2,880 cyclic steps,20 main preparations and54 cyclic
preparations. These are original-instruction observations, not a completed
portable envelope implementation. Live curve-buffer identity and full
gate/tempo scheduling still need verification.

## Voice and scheduler boundaries

The actual `Voice<32,96000,2>` constructor is `0x1802e5cd0`, with object stride
`0x1a000`; `0x1802e6fb0` instead builds an oscillator bank. Six constructor
calls in `0x1802deaf0` receive shared parameter mappings.

- `0x1802cd610` initializes oscillator processors, mixer and envelope arrays.
- `0x1802cd760` prepares voice controls and oscillator type selection.
- `0x1802d7fa0` advances the two main ADSR copies, cycling children and LFOs.
- `0x1802d81b0` processes oscillator1 and oscillator2, conditional cross-routing,
  interpolated levels and their sum. Remaining downstream placement is open.
- `0x1802be9b0` is the containing engine audio/control routine and calls this
  voice renderer in a six-voice loop. Its selected display index below6 reads
  oscillator1; indices6..11 read oscillator2 of voice(index%6), using the
  same voice base and stride. This does not demonstrate a second voice bank.
  Full allocator modes and exposed polyphony still require their own evidence.

Correction: a locally named `routing-voice-prepare.json` response at
`0x1802d7c20` contains a small sorted-pair helper, not voice preparation.
The response named `routing-voice-audio.json` contains the control preparer.
Use addresses and actual evidence rather than those provisional filenames.

Full allocator/event scheduling, modulation, remaining oscillators, effects,
sequencer, state/preset behavior, Android audio/MIDI/UI and instrument-wide
comparisons remain required. No APK or full1:1 result exists yet.
