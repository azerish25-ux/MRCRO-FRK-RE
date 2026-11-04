# Original UI reconstruction evidence

This records packaged declarations and exact asset extraction, not a completed
Android renderer or proof of runtime gesture/pixel equivalence. Full local maps
and original assets remain under ../local/evidence/ui-resource-agent on D:.

## Scene geometry and state
- Logical width 3200. Home content height 1350; Advanced/Sequencer content height
  1878. Top/bottom toolbars add 115 and 75: complete canvases 3200x1540/3200x2068.
- Desktop 100% is logical scale 0.5, with separately selected 100% assets.
- SelectedScreen 0/1/2 selects Home/Advanced/Sequencer; OpenAdvancedPanel follows
  0/1/1. Home/Advanced share SynthView. The lower panel moves y=710/1240 and the
  background y=-528/0; Home forces SynthSelectedBottomPanel=0.
- The expanded map retains 22,580 declaration instances from 17 selected resource
  documents. All selected numeric loops expand after resolving MACRO_NB=4.
  Conditional/action ordering and actual visibility remain unevaluated.

## Original drawable resources
- 699 hashed 100% bitmap, SVG and font resources are indexed with source references.
- Active silver/orange synth knobs are procedural softknobs: gradients, ring/center
  size, shadows, indicator, detents and text follow XML styles. Legacy 128-image
  knob families must not replace those active controls without usage evidence.
- Fonts include Interstate Condensed Regular/Bold, Interstate Light Regular,
  Inter-ExtraLight and Arturia.ttf glyph icons.
- Five ASTR physical-wheel containers were decoded into 640 byte-exact PNG slices.
  Every PNG's CRC, decompression, dimensions and extracted hash passed. No image
  re-encoding was used. Header: ASTR + LE u32 version/count/width/height, then
  count+1 absolute LE offsets; adjacent ranges are original PNG frames.
- Runtime parameter-to-frame rounding, orientation and interpolation are not
  established by container extraction alone.

## Skin-dependent behavior
- mf-37: IS_MINIFREAK_49=0; 37 displayed keys starting MIDI48, bound to the exact
  parameter name KeyboardNote49; static wheel artwork plus 13 LED states.
- mf-49: IS_MINIFREAK_49=1; 49 keys starting MIDI36, bound to KeyboardNote37;
  physical ASTR wheels, filter FM and extra stereo/pan controls.
- GUI-loaded declarations write MiniFreakDualVCF_Mode from IS_MINIFREAK_49.
  Preserve that declared parameter-state effect; no live mf-49 screenshot or
  instantaneous skin-change behavior is claimed.
- Four macro slots are declared. That does not mean all four are simultaneously
  visible as wheel controls. Sequencer declarations include 64x6 note slots and
  exact legacy IDs such as Seq_CurentStep and Seq_CurentPositionInStep.

## Remaining implementation/verification
Implement original layout/include/expression/action semantics and control bindings,
then native widget rendering and interaction: softknob rasterization, fonts/scale
rounding, feedback buffers, filter/FX/LFO/envelope curves, ADSR dragging, sequencer
quantization/zoom, double-click/default reset, sample pickers/drop, macro editor and
initialization ordering. Compare original state sequences and appropriately scaled
captures; existing bounded JPEG baselines alone cannot prove lossless pixel parity.


## Remote Android comparison foundation

Run [41860166281](https://github.com/azerish25-ux/MRCRO-FRK-RE/actions/runs/41860166281)
built and installed actual app/test APKs, passed lint and all five native-window
screenshot checks on API35 x86_64. The source-controlled project acquires715
hash-verified original resources remotely and packages only its three required
assets. Four assigned states produce distinct images; repeated factory values
produce identical pixels. Actual first-draw synchronization, window focus and
first-use immersive tutorial setup prevent obscured/empty captures.

This covers three filter knobs in a test-only app with no synth engine or
launcher flow. RasterConvention explicitly records candidate interpretations;
they are not accepted as native equivalence. Exact native SoftKnob properties,
paint geometry and SliderComposer magnetic/gesture behavior are being traced in
the matched installed wrapper. Full Home/Advanced/Sequencer and all interactions
remain required. The user requires the same UI/UX, including tiny details;
a stylistic approximation is not an acceptance criterion.

See [native runtime survey](native-runtime-ui-survey.md) for subsequently observed
model controls, menus, gestures, original descriptor counts and state restoration.
