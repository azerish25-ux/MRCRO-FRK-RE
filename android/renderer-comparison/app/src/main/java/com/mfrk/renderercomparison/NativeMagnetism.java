package com.mfrk.renderercomparison;

/** Source control math. Pointer sensitivity, clamping and setter dispatch belong to the caller. */
public final class NativeMagnetism {
    static { System.loadLibrary("mfrk_knob_geometry"); }
    private NativeMagnetism() { }
    /** Zones are [positionFrom,positionTo,valueFrom,valueTo,skip] records.
     *  Returns raw float bits in the low word and the native skip flag in bit32.
     *  Values/deltas are in view space. This bridge admits finite normalized,
     *  ordered zone records, at most128; wrapped periods must be positive. */
    public static native long delta(float[] zones, float value, float change,
        boolean flag20, boolean flag21, boolean wrap, boolean matches);
    /** Raw source phase numbers, not Android MotionEvent actions. Current is the
     *  raw parameter value; reverse is applied natively. Returns held-value bits
     *  plus flag20/flag21 in bits32/33. */
    public static native long phase(int phase, boolean matches, boolean reverse,
        float current, float held, boolean flag20, boolean flag21);
    /** Current is raw parameter space. Bit32 means handled, bit33 means a target
     *  exists; only then do the low float bits contain a view-space target.
     *  A handled event with no target must not change the parameter. */
    public static native long wheel(float[] zones, float current, float vertical,
        boolean wheelReversed, boolean inertial, boolean reverse);
    public static float value(long packed) { return Float.intBitsToFloat((int)packed); }
    public static boolean skip(long packed) { return (packed & (1L << 32)) != 0; }
    public static boolean handled(long packed) { return (packed & (1L << 32)) != 0; }
    public static boolean hasTarget(long packed) { return (packed & (1L << 33)) != 0; }
}
