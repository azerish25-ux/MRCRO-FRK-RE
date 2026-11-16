package com.mfrk.renderercomparison;

/** Debug-only observer profile for Android/core routing verification.
 * These literal metadata are test inputs, not recovered widget/controller data. */
final class NativeGestureProbe {
    static { System.loadLibrary("mfrk_knob_geometry"); }
    private NativeGestureProbe() { }
    static native long create(float initial);
    /** Current value, prepare count, end-edit count and parameter-write count. */
    static native float[] snapshot(long attachment);
    /** Total end-edit and destroyed-owner observers across debug probes. */
    static native int[] counters();
}
