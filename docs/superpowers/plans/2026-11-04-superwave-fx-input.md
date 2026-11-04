# SuperWave and dry FX-input integration

Integrate the two independently reconstructed/reviewed candidates, preserving
the actual native32-frame oscillator and16-frame seven-mono-lane gain/pan boundary.
This is required voice-path work; it does not substitute for the complete app.

- [x] Original SuperWave bank/control/pitch/phase/constructor/render evidence,
  independent scalar/C++ and semantic negatives; review complete.
- [x] Original FX-input/Q14 prefix evidence, independent C++ and negatives;
  review complete. Cold history and sum/override follow-up still being joined.
- [ ] Adapt production names/source, remove mutation branches, retain admission.
- [ ] Preserve complete original fixture bytes with bounded, checked readers.
- [ ] Remote failing-stage check, then native/Android exact comparisons.
- [ ] Integrate reviewed changes, continue actual full voice/FX/MIDI/UI graph.

SuperWave has seven uint32 phases, wrapped int16 interpolation/weights, int64
sum, shared verified wave tables, exact LPF/control grouping and float-phase
setter/default. All native32 methods, no invented16 adapter. Table owner lifetime
and representable float conversions remain explicit. Native input words/dirty are
caller-supplied; first prepare establishes weights rather than guessed defaults.

FX prefix has explicit history/settings and seven mono inputs. Do not invent
eighth-input or full effect behavior; use follow-up original sum/ctor evidence
before extending the boundary. Android math imports must match expected bits.

All project files/temp onD:,5-second retry for storage errors. Android SDK and
emulators only on Actions. No vendor app changes or new synth instances.
