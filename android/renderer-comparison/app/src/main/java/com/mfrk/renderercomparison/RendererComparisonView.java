package com.mfrk.renderercomparison;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.view.View;
import org.json.JSONObject;
import org.json.JSONArray;

/** Explicit comparison crop of three original filter controls, without a DSP mock. */
public final class RendererComparisonView extends View {
    private final SourceScene source;
    private final SilverSoftKnob[] widgets = new SilverSoftKnob[3];
    private final RectF[] bounds = new RectF[3], labels = new RectF[3];
    private final Paint labelPaint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.SUBPIXEL_TEXT_FLAG);
    private final Paint bitmapPaint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.FILTER_BITMAP_FLAG);
    private final float[] parameters = new float[3];
    private final String[] centerTexts = new String[]{"", "", ""};
    private float scale = 1, offsetX = 0, offsetY = 0;
    private int drawnFrames;

    public RendererComparisonView(Context context) {
        super(context);
        source = new SourceScene(context);
        setLayerType(View.LAYER_TYPE_SOFTWARE, null); // Explicit deterministic Skia comparison convention.
        setClickable(false); setFocusable(false);
        setContentDescription("Filter control renderer comparison");
        try {
            for (int i=0; i<3; ++i) {
                JSONObject node = source.knobs[i];
                bounds[i] = SourceScene.rectangle(node.getJSONArray("bounds"));
                labels[i] = SourceScene.rectangle(node.getJSONObject("label").getJSONArray("bounds"));
                widgets[i] = new SilverSoftKnob(node.getJSONObject("style"), bounds[i].width());
                parameters[i] = (float)node.getDouble("factory_normalized");
            }
        } catch (Exception e) { throw new IllegalStateException("Source knobs", e); }
    }
    public void assignNormalized(float cutoff, float resonance, float envelope) {
        float[] next = {cutoff, resonance, envelope};
        for (float value : next) if (Float.isNaN(value) || Float.isInfinite(value) || value<0 || value>1)
            throw new IllegalArgumentException("Comparison parameter must be finite normalized0..1");
        System.arraycopy(next,0,parameters,0,3); invalidate();
    }
    public void assignExplicitCenterTexts(String cutoff, String resonance, String envelope) {
        centerTexts[0]=cutoff; centerTexts[1]=resonance; centerTexts[2]=envelope; invalidate();
    }
    @Override protected void onSizeChanged(int width,int height,int oldWidth,int oldHeight) {
        scale = Math.min(width/source.crop.width(), height/source.crop.height());
        offsetX = (width-source.crop.width()*scale)*0.5f;
        offsetY = (height-source.crop.height()*scale)*0.5f;
    }
    @Override protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        canvas.drawColor(SourceScene.color("2e3236"));
        canvas.save(); canvas.translate(offsetX,offsetY); canvas.scale(scale,scale);
        canvas.clipRect(0,0,source.crop.width(),source.crop.height());
        canvas.translate(-source.crop.left,-source.crop.top);
        canvas.drawBitmap(source.background,null,source.backgroundBounds,bitmapPaint);
        try {
            for (int i=0;i<3;++i) {
                // Endpoints/center vectors are unambiguous. General native magnetic
                // inverse mapping is deliberately not guessed by this drawing primitive.
                widgets[i].draw(canvas,bounds[i].left,bounds[i].top,parameters[i],centerTexts[i],source.valueFont);
                JSONObject label = source.knobs[i].getJSONObject("label");
                labelPaint.setColor(SourceScene.color(label.getString("color")));
                labelPaint.setTypeface(source.labelFont); labelPaint.setTextSize((float)label.getDouble("font_size"));
                labelPaint.setTextAlign(Paint.Align.CENTER);
                Paint.FontMetrics metrics = labelPaint.getFontMetrics();
                float baseline = labels[i].centerY()-(metrics.ascent+metrics.descent)*0.5f;
                canvas.drawText(label.getString("text"),labels[i].centerX(),baseline,labelPaint);
            }
        } catch (Exception e) { throw new IllegalStateException("Source label data",e); }
        canvas.restore(); ++drawnFrames;
    }
    public int drawnFrames() { return drawnFrames; }
    public RectF contentPixels() {
        return new RectF(offsetX,offsetY,offsetX+source.crop.width()*scale,offsetY+source.crop.height()*scale);
    }
    public JSONObject audit() {
        try {
            JSONObject result=new JSONObject();result.put("raster_convention",RasterConvention.ID);
            result.put("source_crop",source.document.getJSONArray("crop"));
            result.put("source_evidence_status",source.document.getString("evidence_status"));
            result.put("logical_to_pixel_scale",scale);
            JSONArray assignments=new JSONArray();
            for (int i=0;i<3;++i) {
                JSONObject n=new JSONObject();n.put("parameter",source.knobs[i].getString("parameter"));
                n.put("normalized",parameters[i]);n.put("indicator_position",parameters[i]);
                n.put("source_id",source.knobs[i].getString("source_id"));
                n.put("source_bounds",source.knobs[i].getJSONArray("bounds"));
                n.put("center_text",centerTexts[i]);assignments.put(n);
            }
            result.put("assignments",assignments);result.put("drawn_frames",drawnFrames);
            result.put("value_mapping","explicit normalized endpoints/center; arbitrary native magnetic mapping unverified");
            result.put("gesture_dispatch","not implemented in comparison harness");return result;
        } catch (Exception e) { throw new IllegalStateException("Comparison metadata",e); }
    }
}
