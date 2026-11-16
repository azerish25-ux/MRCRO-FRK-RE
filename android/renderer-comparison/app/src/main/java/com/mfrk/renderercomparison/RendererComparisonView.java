package com.mfrk.renderercomparison;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.view.View;
import android.view.MotionEvent;
import org.json.JSONObject;
import org.json.JSONArray;

/** Explicit comparison crop of three original filter controls, without a DSP mock. */
public final class RendererComparisonView extends View {
    private final SourceScene source;
    private final SilverSoftKnob[] widgets = new SilverSoftKnob[3];
    private final MagneticControl[] mappings = new MagneticControl[3];
    private final RectF[] bounds = new RectF[3], labels = new RectF[3];
    private final Paint labelPaint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.SUBPIXEL_TEXT_FLAG);
    private final Paint bitmapPaint = new Paint(Paint.ANTI_ALIAS_FLAG | Paint.FILTER_BITMAP_FLAG);
    private final float[] parameters = new float[3];
    private final String[] centerTexts = new String[]{"", "", ""};
    private float scale = 1, offsetX = 0, offsetY = 0;
    private int drawnFrames;
    private final SourceTouchRouter.Target[] touchTargets=new SourceTouchRouter.Target[3];
    private final SourceTouchRouter touchRouter=new SourceTouchRouter(new SourceTouchRouter.Surface() {
        @Override public SourceTouchRouter.Target hit(float x,float y) {
            if (scale<=0) return null;
            float sx=(x-offsetX)/scale+source.crop.left,sy=(y-offsetY)/scale+source.crop.top;
            for (int i=touchTargets.length-1;i>=0;--i)
                if (touchTargets[i]!=null && bounds[i]!=null && bounds[i].contains(sx,sy)) return touchTargets[i];
            return null;
        }
        @Override public void captureChanged(boolean captured) {
            if (getParent()!=null) getParent().requestDisallowInterceptTouchEvent(captured);
        }
    });

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
                // Explicit comparison initial state; factory lifecycle is separate.
                mappings[i] = new MagneticControl(node.getJSONArray("zones"), -1f, false, false);
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
    /** Logical source phase only; MotionEvent dispatch remains a separate join. */
    public void controlPhase(int index, int phase, boolean matches, boolean reverse) {
        mappings[index].phase(phase, matches, parameters[index], reverse);
    }
    /** Returns the native view-space result; caller owns clamp/reverse/setter policy. */
    public long mapControlDelta(int index, float delta, boolean reverse, boolean wrap, boolean matches) {
        float current = reverse ? 1f - parameters[index] : parameters[index];
        return mappings[index].delta(current, delta, wrap, matches);
    }
    public long mapControlWheel(int index, float vertical, boolean wheelReversed, boolean inertial, boolean reverse) {
        return mappings[index].wheel(parameters[index], vertical, wheelReversed, inertial, reverse);
    }
    /** Borrow actual controller targets. Replacing a binding first ends captures;
     * the application owns target lifetimes and closes native attachments. */
    public void setTouchTargets(SourceTouchRouter.Target[] targets) {
        if (targets!=null && targets.length!=touchTargets.length) throw new IllegalArgumentException("Three source controls required");
        if (targets!=null) for (int i=0;i<targets.length;++i) for (int j=0;j<i;++j)
            if (targets[i]!=null && targets[i]==targets[j]) throw new IllegalArgumentException("Independent controller per widget required");
        touchRouter.cancel();
        for (int i=0;i<touchTargets.length;++i) touchTargets[i]=targets==null?null:targets[i];
    }
    public NativeSliderController.Coordinates controlCoordinates(int index) {
        if (index<0 || index>=bounds.length) throw new IllegalArgumentException("Source control index");
        return new NativeSliderController.Coordinates() {
            @Override public float localX(float x,float y) { return (x-offsetX)/scale+source.crop.left-bounds[index].left; }
            @Override public float localY(float x,float y) { return (y-offsetY)/scale+source.crop.top-bounds[index].top; }
        };
    }
    public RectF controlPixels(int index) {
        if (index<0 || index>=bounds.length) throw new IllegalArgumentException("Source control index");
        RectF b=bounds[index];
        return new RectF(offsetX+(b.left-source.crop.left)*scale,offsetY+(b.top-source.crop.top)*scale,
            offsetX+(b.right-source.crop.left)*scale,offsetY+(b.bottom-source.crop.top)*scale);
    }
    @Override public boolean onTouchEvent(MotionEvent event) {
        boolean handled=touchRouter.onTouchEvent(event);
        if (handled && touchRouter.completedTap()) performClick();
        return handled || super.onTouchEvent(event);
    }
    @Override public boolean performClick() { super.performClick();return true; }
    @Override public void onWindowFocusChanged(boolean focused) {
        if (!focused) touchRouter.cancel();super.onWindowFocusChanged(focused);
    }
    @Override protected void onDetachedFromWindow() { touchRouter.cancel();super.onDetachedFromWindow(); }
    @Override protected void onSizeChanged(int width,int height,int oldWidth,int oldHeight) {
        touchRouter.cancel();
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
                // Explicit comparison assignments feed the recovered marker geometry.
                // Full native getter/paint lifecycle remains a separate join.
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
                n.put("magnetic_state",mappings[i].audit());
            }
            result.put("assignments",assignments);result.put("drawn_frames",drawnFrames);
            result.put("value_mapping","explicit normalized endpoints/center; arbitrary native magnetic mapping unverified");
            int bound=0;for (SourceTouchRouter.Target target:touchTargets) if (target!=null) ++bound;
            result.put("gesture_dispatch","Android capture adapter; actual controller attachments are supplied by host");
            result.put("bound_touch_controls",bound);result.put("captured_touch_pointers",touchRouter.capturedPointers());return result;
        } catch (Exception e) { throw new IllegalStateException("Comparison metadata",e); }
    }
}
