package com.mfrk.renderercomparison;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Typeface;
import android.graphics.RectF;
import org.json.JSONArray;
import org.json.JSONObject;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;

/** Immutable, source-linked declaration metadata; contains no native widget guesswork. */
final class SourceScene {
    final JSONObject document;
    final RectF crop;
    final RectF backgroundBounds;
    final Bitmap background;
    final Typeface labelFont;
    final Typeface valueFont;
    final JSONObject[] knobs = new JSONObject[3];

    SourceScene(Context context) {
        try {
            try (InputStream in = context.getAssets().open("filter-slice.json")) {
                document = new JSONObject(new String(readBytes(in), StandardCharsets.UTF_8));
            }
            crop = rectangle(document.getJSONArray("crop"));
            JSONObject bg = document.getJSONObject("background");
            backgroundBounds = rectangle(bg.getJSONArray("bounds"));
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inScaled = false;
            options.inPreferredConfig = Bitmap.Config.ARGB_8888;
            try (InputStream in = context.getAssets().open(bg.getString("asset"))) {
                background = BitmapFactory.decodeStream(in, null, options);
            }
            if (background == null) throw new IllegalStateException("Required original background failed to decode");
            labelFont = Typeface.createFromAsset(context.getAssets(), "vendor/fonts/Interstate Condensed Regular.otf");
            valueFont = Typeface.createFromAsset(context.getAssets(), "vendor/fonts/Inter-ExtraLight.ttf");
            JSONArray list = document.getJSONArray("knobs");
            if (list.length() != knobs.length) throw new IllegalArgumentException("Expected source filter slice");
            for (int i = 0; i < knobs.length; ++i) knobs[i] = list.getJSONObject(i);
        } catch (Exception e) {
            throw new IllegalStateException("Source scene or verified assets unavailable", e);
        }
    }
    private static byte[] readBytes(InputStream in) throws java.io.IOException {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        byte[] block = new byte[4096]; int count;
        while ((count = in.read(block)) != -1) out.write(block, 0, count);
        return out.toByteArray();
    }
    static RectF rectangle(JSONArray a) {
        try {
            float x = (float)a.getDouble(0), y = (float)a.getDouble(1);
            return new RectF(x, y, x + (float)a.getDouble(2), y + (float)a.getDouble(3));
        } catch (Exception e) { throw new IllegalArgumentException("Source rectangle", e); }
    }
    static int color(String hex) {
        long value = Long.parseLong(hex, 16);
        return (int)(hex.length() <= 6 ? value | 0xff000000L : value);
    }
}
