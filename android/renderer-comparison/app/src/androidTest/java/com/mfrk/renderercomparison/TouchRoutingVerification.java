package com.mfrk.renderercomparison;

import android.graphics.RectF;
import android.os.SystemClock;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;

/** MotionEvent -> source-coordinate routing -> real native controller, with an
 * explicitly synthetic observer host. Does not assert widget metadata/DSP parity. */
final class TouchRoutingVerification {
    private static long time=SystemClock.uptimeMillis(),downTime=time;
    private static void require(boolean value,String message) { if (!value) throw new AssertionError(message); }
    private static MotionEvent event(int action,int[] ids,float[] xs,float[] ys,int meta) {
        if ((action&MotionEvent.ACTION_MASK)==MotionEvent.ACTION_DOWN) downTime=++time;
        MotionEvent.PointerProperties[] properties=new MotionEvent.PointerProperties[ids.length];
        MotionEvent.PointerCoords[] coords=new MotionEvent.PointerCoords[ids.length];
        for (int i=0;i<ids.length;++i) {
            properties[i]=new MotionEvent.PointerProperties();properties[i].id=ids[i];properties[i].toolType=MotionEvent.TOOL_TYPE_FINGER;
            coords[i]=coordinate(xs[i],ys[i]);
        }
        return MotionEvent.obtain(downTime,++time,action,ids.length,properties,coords,meta,0,1,1,0,0,InputDevice.SOURCE_TOUCHSCREEN,0);
    }
    private static MotionEvent.PointerCoords coordinate(float x,float y) {
        MotionEvent.PointerCoords c=new MotionEvent.PointerCoords();c.x=x;c.y=y;c.pressure=1;c.size=1;return c;
    }
    private static void send(SourceTouchRouter router,int action,int[] ids,float[] xs,float[] ys,int meta) {
        MotionEvent e=event(action,ids,xs,ys,meta);try { require(router.onTouchEvent(e),"Captured stream delivery"); }finally { e.recycle(); }
    }
    private static NativeSliderController target(long handle,float originX) {
        return new NativeSliderController(handle,new NativeSliderController.Coordinates() {
            @Override public float localX(float x,float y) { return x-originX; }
            @Override public float localY(float x,float y) { return y; }
        },null);
    }
    static void verify(RendererComparisonView view) {
        float[] ties={-2.5f,-1.5f,-.5f,-0f,0f,.5f,1.5f,2.5f};int[] rounded={-2,-2,0,0,0,0,2,2};
        for (int i=0;i<ties.length;++i) require(SourceTouchRouter.sourceCoordinate(ties[i])==rounded[i],"Source coordinate tie "+i);
        long firstHandle=NativeGestureProbe.create(.5f),secondHandle=NativeGestureProbe.create(.5f);
        try (NativeSliderController first=target(firstHandle,0);NativeSliderController second=target(secondHandle,200)) {
            int[] captures={0,0};
            SourceTouchRouter router=new SourceTouchRouter(new SourceTouchRouter.Surface() {
                @Override public SourceTouchRouter.Target hit(float x,float y) { return x<100?first:x>=200&&x<300?second:null; }
                @Override public void captureChanged(boolean active) { ++captures[active?0:1]; }
            });
            send(router,MotionEvent.ACTION_DOWN,new int[]{7},new float[]{10.5f},new float[]{20.5f},0);
            send(router,MotionEvent.ACTION_POINTER_DOWN|(1<<MotionEvent.ACTION_POINTER_INDEX_SHIFT),new int[]{7,11},new float[]{10.5f,210.5f},new float[]{20.5f,20.5f},0);
            require(router.capturedPointers()==2,"Independent controls capture two fingers");
            // Android pointer indexes reorder while IDs retain their owners.
            send(router,MotionEvent.ACTION_MOVE,new int[]{11,7},new float[]{210.5f,10.5f},new float[]{10.5f,.5f},0);
            require(Float.floatToRawIntBits(NativeGestureProbe.snapshot(firstHandle)[0])==0x3f19999a,"First native drag value");
            require(Float.floatToRawIntBits(NativeGestureProbe.snapshot(secondHandle)[0])==0x3f0ccccd,"Second native drag value");
            send(router,MotionEvent.ACTION_POINTER_UP|(1<<MotionEvent.ACTION_POINTER_INDEX_SHIFT),new int[]{11,7},new float[]{210.5f,10.5f},new float[]{10.5f,.5f},0);
            require(router.capturedPointers()==1 && NativeGestureProbe.snapshot(firstHandle)[2]==1 && NativeGestureProbe.snapshot(secondHandle)[2]==0,"Lift changes only owning controller");
            send(router,MotionEvent.ACTION_MOVE,new int[]{11},new float[]{210.5f},new float[]{-9.5f},0);
            send(router,MotionEvent.ACTION_CANCEL,new int[]{11},new float[]{210.5f},new float[]{-9.5f},0);
            float[] firstState=NativeGestureProbe.snapshot(firstHandle),secondState=NativeGestureProbe.snapshot(secondHandle);
            require(firstState[2]==1 && firstState[3]==1 && secondState[2]==1 && secondState[3]==2,"Cancel ends edits once without extra writes");
            require(router.capturedPointers()==0 && captures[0]==1 && captures[1]==1,"Capture parent lifecycle");
        }
        verifyHistoryAndModifiers();
        verifyViewCoordinates(view);
        verifyNativeOwnerTeardown();
    }
    private static void verifyHistoryAndModifiers() {
        long handle=NativeGestureProbe.create(.5f);
        try (NativeSliderController target=target(handle,0)) {
            SourceTouchRouter router=new SourceTouchRouter(new SourceTouchRouter.Surface() {
                @Override public SourceTouchRouter.Target hit(float x,float y) { return target; }
                @Override public void captureChanged(boolean active) { }
            });
            send(router,MotionEvent.ACTION_DOWN,new int[]{3},new float[]{10},new float[]{100},0);
            MotionEvent batch=event(MotionEvent.ACTION_MOVE,new int[]{3},new float[]{10},new float[]{90},0);
            batch.addBatch(++time,new MotionEvent.PointerCoords[]{coordinate(10,80)},0);
            try { require(batch.getHistorySize()==1 && router.onTouchEvent(batch),"Batched historical delivery"); }finally { batch.recycle(); }
            float[] state=NativeGestureProbe.snapshot(handle);
            require(state[3]==2 && Float.floatToRawIntBits(state[0])==0x3f19999a,"Both native historical movements execute");
            send(router,MotionEvent.ACTION_MOVE,new int[]{3},new float[]{10},new float[]{80},KeyEvent.META_CTRL_ON);
            require(NativeGestureProbe.snapshot(handle)[0]==state[0],"Fine modifier change at rest has no jump");
            send(router,MotionEvent.ACTION_MOVE,new int[]{3},new float[]{10},new float[]{70},KeyEvent.META_CTRL_ON);
            require(NativeGestureProbe.snapshot(handle)[0]>.609f && NativeGestureProbe.snapshot(handle)[0]<.611f,"Fine modifier reaches source sensitivity");
            send(router,MotionEvent.ACTION_UP,new int[]{3},new float[]{10},new float[]{70},KeyEvent.META_CTRL_ON);
            require(!router.completedTap() && NativeGestureProbe.snapshot(handle)[2]==1,"Dragged release lifecycle");
        }
    }
    private static void verifyViewCoordinates(RendererComparisonView view) {
        long handle=NativeGestureProbe.create(.5f);
        try (NativeSliderController target=new NativeSliderController(handle,view.controlCoordinates(0),null)) {
            view.setTouchTargets(new SourceTouchRouter.Target[]{target,null,null});
            RectF b=view.controlPixels(0);float x=b.left+b.width()*.4f,y=b.top+b.height()*.4f,upY=y-b.height()*.16f;
            int[] actions={MotionEvent.ACTION_DOWN,MotionEvent.ACTION_MOVE,MotionEvent.ACTION_UP};
            for (int action:actions) {
                MotionEvent e=event(action,new int[]{9},new float[]{x},new float[]{action==MotionEvent.ACTION_DOWN?y:upY},0);
                try { require(view.dispatchTouchEvent(e),"Source view hit and capture"); }finally { e.recycle(); }
            }
            float[] state=NativeGestureProbe.snapshot(handle);
            require(Float.floatToRawIntBits(state[0])==0x3f19999a && state[1]==1 && state[2]==1,"Scaled view coordinates reach native controller");
            view.setTouchTargets(null);
        } finally { view.setTouchTargets(null); }
    }
    private static void verifyNativeOwnerTeardown() {
        int[] before=NativeGestureProbe.counters();
        long handle=NativeGestureProbe.create(.5f);NativeSliderController target=target(handle,0);
        target.down(10,20,16);target.close();target.close();
        int[] after=NativeGestureProbe.counters();
        require(after[0]==before[0]+1 && after[1]==before[1]+1,"Detach releases active edit and owner exactly once");
        boolean rejected=false;try { new NativeSliderController(handle,new NativeSliderController.Coordinates() {
            @Override public float localX(float x,float y) { return x; }
            @Override public float localY(float x,float y) { return y; }
        },null); } catch (IllegalArgumentException expected) { rejected=true; }
        require(rejected,"Stale native handles cannot be reused");
    }
}
