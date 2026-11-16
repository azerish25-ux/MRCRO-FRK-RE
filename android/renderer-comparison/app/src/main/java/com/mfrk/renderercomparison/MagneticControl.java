package com.mfrk.renderercomparison;

import org.json.JSONArray;
import org.json.JSONObject;

/** UI-thread control state with source raw phases and resolved literal zone metadata. */
final class MagneticControl {
    private final float[] zones;
    private float held;
    private boolean flag20, flag21;
    MagneticControl(JSONArray definitions, float initialHeld, boolean initial20, boolean initial21) {
        if (definitions.length() > 128) throw new IllegalArgumentException("Zone count");
        zones = new float[definitions.length() * 5];
        held = initialHeld; flag20 = initial20; flag21 = initial21;
        try {
            for (int i = 0; i < definitions.length(); ++i) {
                JSONObject node = definitions.getJSONObject(i);
                zones[i * 5] = Float.parseFloat(node.getString("from"));
                zones[i * 5 + 1] = Float.parseFloat(node.getString("to"));
                if (node.has("val")) {
                    zones[i * 5 + 2] = zones[i * 5 + 3] = Float.parseFloat(node.getString("val"));
                } else {
                    zones[i * 5 + 2] = Float.parseFloat(node.getString("val-from"));
                    zones[i * 5 + 3] = Float.parseFloat(node.getString("val-to"));
                }
                String skip = node.optString("skip-at-drag-start", "0");
                if (!skip.equals("0") && !skip.equals("1")) throw new IllegalArgumentException("Unresolved skip flag");
                zones[i * 5 + 4] = skip.equals("1") ? 1f : 0f;
            }
        } catch (Exception error) { throw new IllegalArgumentException("Resolved literal magnetic zones", error); }
    }
    void phase(int phase, boolean matches, float current, boolean reverse) {
        long result = NativeMagnetism.phase(phase, matches, reverse, current, held, flag20, flag21);
        held = NativeMagnetism.value(result);
        flag20 = (result & (1L << 32)) != 0;
        flag21 = (result & (1L << 33)) != 0;
    }
    long delta(float viewValue, float change, boolean wrap, boolean matches) {
        return NativeMagnetism.delta(zones, viewValue, change, flag20, flag21, wrap, matches);
    }
    JSONObject audit() throws org.json.JSONException {
        JSONObject result = new JSONObject();
        result.put("held", held); result.put("flag20", flag20); result.put("flag21", flag21);
        result.put("zone_count", zones.length / 5); return result;
    }
}
