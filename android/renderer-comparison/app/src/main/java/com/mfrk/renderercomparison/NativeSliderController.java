package com.mfrk.renderercomparison;

import android.os.Looper;

/** Touch target for a native, fully owned source controller attachment.
 * The application supplies the real GUI host and metadata through the native
 * attach_controller API. This class never invents parameter metadata or setters. */
public final class NativeSliderController implements SourceTouchRouter.Target, AutoCloseable {
    public interface Coordinates {
        float localX(float viewX, float viewY);
        float localY(float viewX, float viewY);
    }
    static { System.loadLibrary("mfrk_knob_geometry"); }
    private long handle;
    private final Coordinates coordinates;
    private final Runnable changed;
    public NativeSliderController(long attachedHandle, Coordinates coordinates, Runnable changed) {
        requireMainThread();
        if (coordinates==null || !isAttachedNative(attachedHandle))
            throw new IllegalArgumentException("An owned native controller and coordinates are required");
        this.handle=attachedHandle;this.coordinates=coordinates;this.changed=changed;
    }
    private static void requireMainThread() {
        if (Looper.myLooper()!=Looper.getMainLooper()) throw new IllegalStateException("Controller input requires the UI thread");
    }
    @Override public float localX(float x,float y) { return coordinates.localX(x,y); }
    @Override public float localY(float x,float y) { return coordinates.localY(x,y); }
    private void dispatch(int action,int x,int y,int modifiers) {
        requireMainThread();
        if (handle==0) { if (action>=2) return;throw new IllegalStateException("Detached controller"); }
        dispatchNative(handle,action,x,y,modifiers);
        if (changed!=null) changed.run();
    }
    @Override public void down(int x,int y,int modifiers) { dispatch(0,x,y,modifiers); }
    @Override public void drag(int x,int y,int modifiers) { dispatch(1,x,y,modifiers); }
    @Override public void up(int x,int y,int modifiers,boolean cancelled) { dispatch(cancelled?3:2,x,y,modifiers); }
    @Override public void close() {
        requireMainThread();long closing=handle;handle=0;
        if (closing!=0) detachNative(closing);
    }
    private static native boolean isAttachedNative(long handle);
    private static native void dispatchNative(long handle,int action,int x,int y,int modifiers);
    private static native void detachNative(long handle);
}
