package com.mfrk.renderercomparison;

/** Source-derived geometry; renderer rasterization remains a separate profile. */
public final class NativeGeometry {
    static { System.loadLibrary("mfrk_knob_geometry"); }
    private NativeGeometry() { }
    public static native float[] center(int width, int height, float radius, float center);
    public static native float[] outline(int width, int height, float angle, float gradient, float radius);
    public static native float[] marker(int width, int height, float position, float angle, float radius, float start, float markerWidth);
}
