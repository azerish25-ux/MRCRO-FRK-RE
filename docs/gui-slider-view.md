# Slider view state and modulation display

`GuiSliderViewObserver` follows the actual filter widget observer methods
`d4c300` / `c450d0`. Ordinary changes refresh the selected parameter's cached
view value, optional movement, parent geometry, invalidation and normalized-view
signal, then update its display feature even when another parameter changed.
Origin-6 scalar delivery writes its separate raw cache without an invented
parameter write or repaint.

`GuiControlFeature` reconstructs `f55d60` / `f56450`: scalar display updates,
bounded history, float/byte packets, byte slicing, extrema/positions, selected
mean windows, inversion, active-state changes and component-state requests.
It retains the native extrema scan excluding sample 0, newest-first tie order,
packed eight-sample accumulation and callback timing. Scalar feature callbacks
receive old/new values before the mean is stored; range callbacks follow the
stored update. Disabling a buffered feature clears history and sets only its
min/max/mean to -1, preserving position fields.

`GuiControlParameters` uses the observer's actual row list, including borrowed
registered controller pointers. It retains indexed lookup, ID lookup, preferred
index override, invalid ID sentinel and explicit missing-controller errors.
Controller data access borrows the actual bank value bytes, including scalar
bytes where requested; data classification uses the descriptor type byte.
Descriptor type now lives in the common GUI metadata rather than a separate
write-policy shadow field.

Read-only inspection found that the actual cutoff widget has primary ID 13 and
monitor ID `0x8000095a`, a 352-byte offset / 32-byte packet slice, 64-sample history
and latest-32 mean. Its configured fields are in
`resources/filter-slider-bindings.json`. Runtime history/cache contents are not
presented as constructor defaults. The live knob and its wrapper are 100x100;
conversion from the source's 125x125 declarations remains a separate calibration.

The comparison executes the native observer, feature/history/statistics,
controller lookup and scalar/data/descriptor methods. Backend value lookup,
geometry/smart-parent, widget invalidation/range callbacks and signals are
observed endpoints. Portable reads use actual initialized preset-bank values.
All 928 cases and 1,397 events match locally, including the captured cutoff
configuration with synthetic packets, prior native state carried forward,
finite dense arithmetic, callback-time reversal/geometry/signal changes and
raw scalar-cache NaN bits. The hash-bound fixture is configured for Android CI.

The knob's buffered modulation range callback now follows `f67ea0/f681f0`.
It bounds both clamped endpoint points and crossed extrema, retains the raw
range comparisons, expands by half the source stroke plus three pixels, rounds
outward and unions the previous/current integer bounds. Packed trigonometry
retains the different two-angle and scalar entry paths. `GuiKnobFeatureBinding`
connects registered parameter rows and the existing feature history to this
calculation and the component's actual repaint scheduler. The component supplies
current resolved style getters and actual component services.
All 512 native full-callback cases match locally through this binding.

The same binding now executes the scalar `f67c80/f67800/f84300` callback.
Its narrow arc retains the original 0.05-radian line sampling and endpoint
clamping, stored path extrema, full stroke expansion and separate old/new
outward-rounded repaint requests. All 384 native full-callback cases match
locally. This profile requires stable style/dimensions across both scheduler
callbacks.

`GuiKnobComponentDisplayBoundary` joins both dirty callbacks to the source
component tree's `ff6020/ff6120` path. It clips in place to actual visible bounds,
retains zero-area edge locations, checks visibility/cache invalidation, translates
through parents, applies the source affine corner calculation, scales root-peer
requests and schedules the actual peer. Cache invalidation distinguishes
`invalidateAll()` from `invalidate(actualDirtyBounds)`; the partial call receives
the source rectangle. The cache's boolean return determines whether propagation
continues. `COMPRP03` authenticates those payloads; the earlier gate-only
projection omitted them. All 384 cases and 1,847 events match locally, including
three-level trees, absent peers/parents, cache refusal, reflection/shear and
fractional peer dimensions.

The component fixture also executes the knob's actual `b70010/5a0600`
visible-bound methods and `ff5ee0` full repaint. `GuiKnobVisibleBounds` retains
binary64 fractional-bound truncation and unions from registered bounds providers.
The actual cutoff vtable's final override returns an empty rectangle; this
specific method is source-confirmed, not a default for unrelated widget types.
Full repaint subtracts the current component position and invalidates the whole
cache without another clipping call. The knob component boundary owns that full
repaint as well as its range/scalar dirty requests.

`ui_component_visibility.hpp` follows `b6eab0/ff2710/ff2880/ff5740/ff2930/ff20e0`.
It stores requested visibility before running every veto hook, updates the actual
flag only through the admitted transition, repaints, releases cached resources
recursively, routes focus changes and invokes visibility/hierarchy notifications.
Listener traversal retains reverse order and live-size checks. Weak guards retain
their token without retaining the component; expired guards stop subsequent
notifications. Widget signals retain requested-state delivery even after veto,
with transition signals selected from the flag captured at entry. Signal targets
are read at their actual delivery points.

The concrete knob display boundary now invokes this widget state path. Feature
state checks borrow the same component flag byte, preventing a divergent display
copy. The joined native comparison covers 512 cases/1,526 events, including veto,
listener mutations, 94 weak-expiration cases and callback-time signal changes.
The fixture retains component storage while expiring its weak token; it does not
prove final native destructor behavior. Child arrays remain stable during cached
resource release; ordinary children have no desktop-peer flag and parent mapping
remains stable through visible-bound callbacks.

The hierarchy tail resolves the component's actual accessibility handler and
emits a structure-change notification. Source `ff9d80/1076fe0` confirms handler
construction/type ownership and native accessibility-client dispatch. The
platform boundary now exposes typed accessibility events. `COMPVS02` also covers
accessibility being unavailable while a peer is present; handler/event bodies
remain required platform services.

Mouse/focus/accessibility bodies, image-cache implementation, platform peer, registered bounds
providers, style getters and real scene construction remain required services.
Upstream [JUCE Component source](https://github.com/juce-framework/JUCE/blob/6.1.6/modules/juce_gui_basics/components/juce_Component.cpp)
helped identify cache and listener terminology; it does not identify the installed
JUCE version. Original instructions remain the behavioral comparison source.

This is source view state, not a completed widget factory or paint oracle.
Bounds-provider/platform services, timers, scene
construction, native scale/raster calibration and full synth bindings remain
unfinished. Packet data must not alias the feature history; incoming sample
count must fit capacity. Finite view/history values and bounded geometry are
the admitted profile. The actual scene must supply its controller services and
retain referenced objects through callbacks; no successful/default services
are installed here.
