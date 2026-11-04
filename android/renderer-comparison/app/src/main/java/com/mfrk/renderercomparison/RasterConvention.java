package com.mfrk.renderercomparison;

import org.json.JSONObject;

/** Every unresolved native raster convention lives here, not in source metadata. */
final class RasterConvention {
    static final String ID = "source-constrained-skia-candidate-v1";
    private RasterConvention() {}
    static float dimension(JSONObject style, String key) {
        try { return Float.parseFloat(style.getString(key)); }
        catch (Exception e) { throw new IllegalArgumentException("Missing source dimension: " + key, e); }
    }
    // Candidate: native angle is a half-sweep in radians, symmetric about12o'clock.
    static float halfSweepDegrees(JSONObject style) {
        return (float)Math.toDegrees(dimension(style, "angle"));
    }
    // Candidate: fractional thicknesses refer to the declared knob diameter.
    static float thickness(float diameter, JSONObject style, String key) {
        return diameter * dimension(style, key);
    }
    // Candidate: factor12 divides the diameter. Native blur/offset kernel is unknown.
    static float shadowYOffset(float diameter, JSONObject style) {
        float divisor = dimension(style, "shadow-offset-y-factor");
        return divisor == 0 ? 0 : diameter / divisor;
    }
    static float indicatorDegrees(JSONObject style, float position) {
        return -90 + (position * 2 - 1) * halfSweepDegrees(style);
    }
    // Candidate: outline-gradient-angle is radians from the positive x-axis.
    static float gradientAngle(JSONObject style) {
        return dimension(style, "center-outline-gradient-angle");
    }
}
