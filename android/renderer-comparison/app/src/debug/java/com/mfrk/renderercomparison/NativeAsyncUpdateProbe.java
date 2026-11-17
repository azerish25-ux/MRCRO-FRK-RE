package com.mfrk.renderercomparison;

final class NativeAsyncUpdateProbe {
    static { System.loadLibrary("mfrk_knob_geometry"); }
    private NativeAsyncUpdateProbe() {}
    static native long start(int mode);
    static native int[] snapshot(long handle);
    static native void close(long handle);
}
