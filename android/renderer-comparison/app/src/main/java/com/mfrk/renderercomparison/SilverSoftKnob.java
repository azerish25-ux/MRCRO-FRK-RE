package com.mfrk.renderercomparison;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RadialGradient;
import android.graphics.LinearGradient;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.Typeface;
import org.json.JSONObject;

/** Reusable Canvas primitive driven by the original silver/baseknob style chain. */
final class SilverSoftKnob {
    private final JSONObject style;
    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.SUBPIXEL_TEXT_FLAG);
    private final float diameter, radius, centerRadius, centerOutline, ringWidth, halfSweep;
    private final int nativeSize;
    private final float nativeAngle, markerStart, markerWidth;
    private final RectF ring;
    private final Shader ringGradient, outlineGradient;
    private final int background, foreground, center, indicator, ringOutline, shadow;

    SilverSoftKnob(JSONObject style, float diameter) {
        this.style = style;
        this.diameter = diameter;
        nativeSize = Math.round(diameter);
        radius = diameter * 0.5f * RasterConvention.dimension(style, "radius");
        float[] centerBounds = NativeGeometry.center(nativeSize, nativeSize,
            RasterConvention.dimension(style, "radius"), RasterConvention.dimension(style, "center-size"));
        centerRadius = centerBounds[2] * 0.5f;
        nativeAngle = RasterConvention.dimension(style, "angle");
        markerStart = RasterConvention.dimension(style, "center-value-start");
        markerWidth = RasterConvention.dimension(style, "center-value-width");
        centerOutline = RasterConvention.thickness(diameter, style, "center-outline");
        ringWidth = RasterConvention.thickness(diameter, style, "ring-thickness");
        halfSweep = RasterConvention.halfSweepDegrees(style);
        try {
            background = SourceScene.color(style.getString("bg-color"));
            foreground = SourceScene.color(style.getString("fg-color"));
            center = SourceScene.color(style.getString("center-color"));
            indicator = SourceScene.color(style.getString("center-value-color"));
            ringOutline = SourceScene.color(style.getString("ring-outline-color"));
            shadow = SourceScene.color(style.getString("shadow-color"));
            int silver = SourceScene.color(style.getString("ring-gradient-color"));
            ringGradient = new RadialGradient(radius, radius, radius,
                new int[]{background, silver, foreground},
                new float[]{0, RasterConvention.dimension(style, "center-size"), 1}, Shader.TileMode.CLAMP);
            float[] gradient = NativeGeometry.outline(nativeSize, nativeSize, nativeAngle,
                RasterConvention.dimension(style, "center-outline-gradient-angle"), centerRadius);
            outlineGradient = new LinearGradient(gradient[0], gradient[1], gradient[2], gradient[3],
                SourceScene.color(style.getString("center-outline-gradient-start-color")),
                SourceScene.color(style.getString("center-outline-gradient-end-color")), Shader.TileMode.CLAMP);
        } catch (Exception e) { throw new IllegalArgumentException("Original knob style incomplete", e); }
        float inset = ringWidth * 0.5f;
        ring = new RectF(inset, inset, diameter-inset, diameter-inset);
    }

    void draw(Canvas canvas, float x, float y, float position, String centerText, Typeface valueFont) {
        canvas.save();
        canvas.translate(x, y);
        paint.reset(); paint.setAntiAlias(true);
        paint.setStyle(Paint.Style.FILL); paint.setColor(background);
        if (RasterConvention.dimension(style, "center-shadow") != 0) {
            paint.setShadowLayer(ringWidth, 0, RasterConvention.shadowYOffset(diameter, style), shadow);
        }
        canvas.drawCircle(radius, radius, radius-ringWidth, paint);
        paint.clearShadowLayer();
        paint.setStyle(Paint.Style.STROKE); paint.setStrokeWidth(ringWidth);
        paint.setStrokeCap(Paint.Cap.ROUND); paint.setShader(ringGradient);
        canvas.drawArc(ring, -90-halfSweep, 2*halfSweep, false, paint);
        paint.setShader(null); paint.setColor(ringOutline);
        paint.setStrokeWidth(RasterConvention.thickness(diameter, style, "ring-outline-thickness"));
        canvas.drawCircle(radius, radius, radius-ringWidth*0.5f, paint);
        paint.setColor(0xffffffff); paint.setShader(outlineGradient); paint.setStrokeWidth(centerOutline);
        canvas.drawCircle(radius, radius, centerRadius-centerOutline*0.5f, paint);
        paint.setShader(null); paint.setStyle(Paint.Style.FILL); paint.setColor(center);
        canvas.drawCircle(radius, radius, centerRadius-centerOutline, paint);
        paint.setColor(indicator); paint.setStyle(Paint.Style.STROKE); paint.setStrokeCap(Paint.Cap.ROUND);
        paint.setStrokeWidth(RasterConvention.thickness(diameter, style, "center-value-thickness"));
        float[] marker = NativeGeometry.marker(nativeSize, nativeSize, position, nativeAngle,
            centerRadius, markerStart, markerWidth);
        canvas.drawLine(marker[0], marker[1], marker[2], marker[3], paint);
        // Text is explicit harness input; no invented native numeric/unit formatter.
        if (centerText != null && !centerText.isEmpty()) {
            paint.setStyle(Paint.Style.FILL); paint.setTypeface(valueFont);
            paint.setTextAlign(Paint.Align.CENTER); paint.setSubpixelText(true);
            paint.setTextSize(RasterConvention.dimension(style, "fontsize"));
            Paint.FontMetrics f = paint.getFontMetrics();
            canvas.drawText(centerText, radius, radius-(f.ascent+f.descent)*0.5f, paint);
        }
        canvas.restore();
    }
}
