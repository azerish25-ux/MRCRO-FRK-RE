# Native runtime UI survey

## Identity and boundary

The already-open Windows application reports **MiniFreak V 5.1.1.6945 (x64)** in
About. The inspected VST3 SHA-256 is
`354c26c58e4c8c671b50f88d070452b47c9cd6b6505b1ea18391724929b3d19e`.
The user authorized exclusive control for this pass on 2026-11-04. No new vendor
instance was launched. Original binaries were not modified.

Bounded Computer Use screenshots, notes and hashes are retained on D: in
`../local/evidence/ui-runtime-survey/<observation>/observation.json`.
These compressed captures establish observed UI behavior; they are not lossless
pixel goldens. User patch backups and captures remain local.

## Oscillator presentation

At the native enlarged view, all 24 Oscillator 1 choices were visible and each
model's dynamic control labels were inspected. Tune and oscillator Volume are
common. The three model-specific controls are:

| Model | Control 1 | Control 2 | Control 3 |
| --- | --- | --- | --- |
| Basic Waves | Morph | Sym | Sub |
| SuperWave | Wave | Detune | Volume |
| Harmo | Content | Sculpting | Chorus |
| KarplusStr | Bow | Position | Decay |
| VAnalog | Detune | Shape | Wave |
| Waveshaper | Wave | Amount | Asym |
| Two Op. FM | Ratio | Amount | Feedback |
| Formant | Interval | Formant | Shape |
| Speech | Type | Timbre | Word |
| Modal | Inharm | Timbre | Decay |
| Noise | Type | Rate | Balance |
| Bass | Saturate | Fold | Noise |
| SawX | SawMod | Shape | Noise |
| Harm | Spread | Rectify | Noise |
| Audio In | Fold | Decimate | BitCrush |
| Wavetable | Position | Width | Filter |
| Sample | Start | Length | Loop |
| Cloud Grains | Start | Density | Chaos |
| Hit Grains | Start | Density | Shape |
| Frozen | Start | Size | Chaos |
| Skan | Start | Speed | Chaos |
| Particle | Start | Density | Chaos |
| Lick | Start | Size | Chaos |
| Raster | Start | Density | Chaos |

SuperWave therefore has two distinct controls labeled Volume. Audio In's
Decimate and BitCrush appeared grayed in the observed state; Oscillator 2
Destroy's corresponding controls appeared active. Eligibility needs its original
binding conditions, rather than a permanent disabled-state assumption.

Oscillator 1's next arrow wraps Raster to Basic Waves. Oscillator 2 exposes 21
choices, ending Destroy, whose next arrow wraps to Basic Waves. Its seven distinct
choices relative to Oscillator 1 are:

| Model | Control 1 | Control 2 | Control 3 |
| --- | --- | --- | --- |
| Chords | Interval | Inv / Trsp | Waveform |
| FM / RM | Wave | FreqMod | RingMod |
| Multi Filter | Cutoff | Res | Mode |
| Surgeon Filter | Cutoff | Spread | Mode |
| Comb Filter | Cutoff | Gain | Damping |
| Phaser Filter | Cutoff | Feedback | Poles |
| Destroy | Fold | Decimate | BitCrush |

Oscillator 2
does not expose Oscillator 1's Audio In, Wavetable or sample/granular models.

The visible count must not change parameter normalization. The latest XML has
30 Oscillator 2 slots, with nine trailing Dummy strings. A read-only native
descriptor inspection independently confirmed all 30 entries; Oscillator 1 has
24. Preserve the 30-slot domain and separately recover the original menu filter.
Evidence: `native-ui-source/scene-builder/live-oscillator-type-metadata-v1.json`.

## Browsers, gestures and secondary controls

- Sample and wavetable selectors open a category/list popup with Factory,
  Import, close, current-item coloring, a vertical scrollbar and previous/next
  arrows. Opening the sample browser scrolls around the selected sample.
  No replacement sample or wavetable was chosen during this pass.
- The sample Import dialog advertises WAV, AIFF, FLAC and SF, with explicit
  upper/lowercase filters. It was inspected and canceled without importing.
- Macro destination selection and assignable matrix headers enter Learn mode,
  highlighting eligible knobs and matrix cells. Tapping Filter Cutoff assigned a
  macro destination and exited Learn. Escape did not cancel the observed Learn
  mode; tapping the same matrix header again did.
- Macro amount vertical dragging changed a displayed integer amount from 0 to
  47. A matrix CycEnv-to-Pitch amount drag displayed 9.43; double-click reset it
  to 0. These gestures and formatting are distinct.
- The wheel detail panel exposes Vibrato enable, Vibrato Rate, Vibrato Depth and
  Bend Range. It expands over the wheel area and closes through its wrench.
- Chord enable is separate from interval editing. Selecting an exact upper
  interval target while Chord was off produced a native History transaction
  Length 3 to 4 and Offset 0 to 3. Octave navigation changed the displayed note
  set without an undo transaction; octave 0 was restored.
- About dims the background and contains scrollable credits and version.
  Escape did not close the observed overlay; clicking outside did.
- Resize choices are 50, 60, 70, 80, 90, 100, 120, 140, 160, 180 and 200 percent.
  There are no 110 or 130 percent entries. Enlarged views can be clipped by the
  desktop; matching scale is not evidence of an unconstrained canvas.

Use the native renderer and hit-target evidence when implementing touch access.
These observations do not authorize replacing exact controls with generic
dropdowns, changing normalization counts, or treating a screenshot as the scene.

## State preservation and remaining work

The native History checkpoint was used to restore every temporary oscillator,
macro/matrix and chord edit. After the oscillator tests, all 2,488 mapped shared
scalar fields matched the retained native backup bit for bit, explicitly covering
134 LFO/shaper and 1,622 sequence fields. The final view was returned to Home at
80 percent with keyboard, Wheels, octave 0 and no popup. Final read-only comparison at 20:38:48 UTC again found zero differences across
all 2,488 mapped values. All five UI-prefix records matched initial raw bits,
the native root returned to 1280 by 616, and the three initially measured filter
knob bounds/styles matched. Evidence: native-ui-source/scene-builder/
concluding-gui-preservation-v2.json and scalar-backup-comparison-
concluding-gui-history-v10.json. Dynamic DSP histories and opaque/unmapped state
are outside this preservation comparison.

Remaining work includes complete FX and preset-management flows, tutorial
transitions, modifier/precision gestures, hardware-related state, all scene
bindings, font/raster equivalence, touch equivalents and full sounding Android
integration. Inspection of these bounded behaviors does not complete UI parity.
