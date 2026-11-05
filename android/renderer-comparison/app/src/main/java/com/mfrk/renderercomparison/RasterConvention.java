package com.mfrk.renderercomparison;

import org.json.JSONObject;

/** Every unresolved native raster convention lives here, not in source metadata. */
final class RasterConvention {
    static final String ID = "source-geometry-skia-candidate-v2";
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
}
