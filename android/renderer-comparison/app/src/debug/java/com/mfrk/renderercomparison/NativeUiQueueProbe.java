package com.mfrk.renderercomparison;

/** Debug-only recipient observer for real Android main-loop scheduling. */
final class NativeUiQueueProbe {
    static { System.loadLibrary("mfrk_knob_geometry"); }
    private NativeUiQueueProbe() {}
    static native long create(boolean nested);
    static native long postFromWorker(long probe, int count, boolean heapData);
    static native void expire(long probe, boolean listener);
    static native boolean cancel(long message);
    static native int[] snapshot(long probe);
    static native void close(long probe);
}
