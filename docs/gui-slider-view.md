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

This is source view state, not a completed widget factory or paint oracle.
Actual widget-specific dirty-region/visibility-hook bodies, timers, scene
construction, native scale/raster calibration and full synth bindings remain
unfinished. Packet data must not alias the feature history; incoming sample
count must fit capacity. Finite view/history values and bounded geometry are
the admitted profile. The actual scene must supply its controller services and
retain referenced objects through callbacks; no successful/default services
are installed here.
