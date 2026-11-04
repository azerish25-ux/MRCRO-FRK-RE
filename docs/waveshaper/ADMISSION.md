# WaveShaper production admission

`mfrk::waveshaper::Engine::admitted_zero_arena(tables)` explicitly supplies zero preconstructor memory. Native inline construction writes only 52 of 216 source-state bytes. The other 164 zeros are an admitted reference profile, not inferred native heap or prototype-factory defaults. Canonical address words in internal storage are inert labels and are never dereferenced. Tables and their lifetime are owned by the caller.

Call `initialize()` before rendering. Constructor-only zero memory is not universally audio-ready: original pitch-0 rendering before Init performs an unmapped fold lookup. Initialization sets the native inner oscillator fields, but preserves outer upsample history, parameters and gate/latch state. Routed reset and phase operations are no-ops; neither substitutes for initialization.

## Defined native behavior

This fixed Type5 component processes 16 internal samples into 32 outputs at the recovered 96kHz template. Three uint16 words, their dirty bytes, the second pitch argument, gate and output level reproduce the original field boundary. Pitch's first float argument is ignored. The original parameter normalization is exact bits 0x37800200; it is not replaced by division by 65535 or a clamp to one. Its endpoint can select table integral 4, where the sixth source pointer aliases the fifth physical table.

Tables are immutable mathematical data. Construct them outside audio callbacks under nearest-even rounding, masked FP exceptions and gradual underflow. The renderer uses lookup/arithmetic, not imported CRT sine/power calls. Every generated word must pass target-platform gold; host matching is not assumed Android equivalence.

The normal inspected FreakOscSlot dirty-pitch update clamps the final transposed/quantized note to 0..127 before forwarding XMM2. This is evidence for that path only. Alternate/init/bypass paths, complete modulation/smoothing and exception policies remain owning-graph work. Negative pitch or fast morph trajectories are not automatically malformed user actions.

Original direct-engine pitch -48/-119 with shape0/fold65535 and morph jumping 0 to 65535 on the next block produces an unmapped lookup at 0x18032f53c. No native audio/equivalence is claimed after this fault. These original-instruction failures and the initialized, defined MIDI0/12/-24 stress results are retained under provenance/.

## Release-active status contract

Use `try_render_raw`, `try_render_scaled` or `try_process`, passing a caller-owned 32-float array and checking the returned `RenderStatus`. `ok` is the only success result. Other statuses are `not_initialized`, `shape_domain` and `fold_domain`. Table checks remain active with NDEBUG. No fabricated numeric clamp, silence or generic reset is introduced.

Failed render leaves output, all 216 state bytes and both internal trace arrays unchanged. `try_process` also rolls back its input/control stores, Init/prepare and admission flag. Earlier independent setter/prepare calls are outside a later render's transaction and remain as they were at that render's entry. FP exception/status flags are not transactional. Invalid individual setter indices return false without mutation.

This is an explicit portable failure contract outside the original defined lookup domain, not a claim about the native exception handler or fault audio. The owner must choose and report its policy rather than silently treating stale output as a newly synthesized block.

```cpp
mfrk::waveshaper::Tables tables; // Startup, outside audio callbacks.
auto engine = mfrk::waveshaper::Engine::admitted_zero_arena(tables);
engine.initialize();
mfrk::waveshaper::Input input{};
input.pitch_argument = 69.0f;
input.words = {32768, 32768, 32768};
input.dirty = {1, 1, 1};
std::array<float, 32> output{};
const auto status = engine.try_process(input, output);
if (status != mfrk::waveshaper::RenderStatus::ok) {
    // Surface status and apply the owning engine's explicit failure policy.
    // output and engine state are unchanged; this is not synthesized silence.
}
```

This component is not a host MIDI parser, does not assign MIDI/MPE meaning to opaque keys or binding metadata, and does not implement the surrounding voice/filter/effects/preset/UI graph. Root owns target validation and integration of those required parts.
