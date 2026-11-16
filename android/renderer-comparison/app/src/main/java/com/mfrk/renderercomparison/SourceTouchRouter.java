package com.mfrk.renderercomparison;

import android.view.KeyEvent;
import android.view.MotionEvent;
import android.util.SparseArray;
import java.util.ArrayList;

/** Android pointer ownership and source-coordinate conversion. No value mapping
 * or parameter writes are implemented here: targets deliver to real controls. */
public final class SourceTouchRouter {
    public interface Target {
        float localX(float viewX, float viewY);
        float localY(float viewX, float viewY);
        void down(int x, int y, int modifiers);
        void drag(int x, int y, int modifiers);
        void up(int x, int y, int modifiers, boolean cancelled);
    }
    public interface Surface {
        Target hit(float viewX, float viewY);
        void captureChanged(boolean captured);
    }
    private static final class Capture {
        final int id; final Target target;
        float viewX, viewY; int x, y, modifiers; boolean dragged, begun;
        Capture(int id, Target target) { this.id=id; this.target=target; }
    }
    private final Surface surface;
    private final SparseArray<Capture> pointers=new SparseArray<>();
    private final ArrayList<Capture> order=new ArrayList<>();
    private boolean ownedStream, cancelPending, dispatching, completedTap;

    public SourceTouchRouter(Surface surface) {
        if (surface==null) throw new IllegalArgumentException("Touch surface required");
        this.surface=surface;
    }
    /** Source a67f10, under the source default round-to-nearest-even FP mode.
     * Preserve the binary64 add and low-word extraction, including negative ties. */
    public static int sourceCoordinate(float value) {
        if (!Float.isFinite(value) || value < -2147483648f || value >= 2147483648f)
            throw new IllegalArgumentException("Finite signed source coordinate required");
        return (int)Double.doubleToRawLongBits((double)value+6755399441055744.0);
    }
    /** Source JUCE modifier bits, not MotionEvent action numbers. */
    public static int sourceModifiers(MotionEvent event, boolean released) {
        int keys=KeyEvent.normalizeMetaState(event.getMetaState()), result=0;
        if ((keys&KeyEvent.META_SHIFT_ON)!=0) result|=1;
        if ((keys&KeyEvent.META_CTRL_ON)!=0) result|=2;
        if ((keys&KeyEvent.META_ALT_ON)!=0) result|=4;
        if ((keys&KeyEvent.META_META_ON)!=0) result|=8;
        if (!released) {
            int buttons=event.getButtonState();
            if ((buttons&MotionEvent.BUTTON_PRIMARY)!=0) result|=16;
            if ((buttons&MotionEvent.BUTTON_SECONDARY)!=0) result|=32;
            if ((buttons&MotionEvent.BUTTON_TERTIARY)!=0) result|=64;
            // A pressed finger/stylus has no Android mouse-button state.
            if (buttons==0) result|=16;
        }
        return result;
    }
    private void position(Capture c, float x, float y, int modifiers) {
        int localX=sourceCoordinate(c.target.localX(x,y));
        int localY=sourceCoordinate(c.target.localY(x,y));
        c.x=localX;c.y=localY;
        c.viewX=x; c.viewY=y; c.modifiers=modifiers;
    }
    private void start(MotionEvent event) {
        int index=event.getActionIndex(), id=event.getPointerId(index);
        float x=event.getX(index), y=event.getY(index);
        Target target=surface.hit(x,y);
        if (target==null || pointers.get(id)!=null) return;
        for (Capture c:order) if (c.target==target) return; // One finger per control.
        Capture c=new Capture(id,target);position(c,x,y,sourceModifiers(event,false));
        boolean first=order.isEmpty();pointers.put(id,c);order.add(c);ownedStream=true;
        if (first) surface.captureChanged(true);
        c.begun=true;target.down(c.x,c.y,c.modifiers);
    }
    private void move(MotionEvent event, int history) {
        int modifiers=sourceModifiers(event,false);
        for (int index=0;index<event.getPointerCount() && !cancelPending;++index) {
            Capture c=pointers.get(event.getPointerId(index));if (c==null) continue;
            float x=history<0?event.getX(index):event.getHistoricalX(index,history);
            float y=history<0?event.getY(index):event.getHistoricalY(index,history);
            if (x==c.viewX && y==c.viewY && modifiers==c.modifiers) continue;
            position(c,x,y,modifiers);c.dragged=true;c.target.drag(c.x,c.y,c.modifiers);
        }
    }
    private void finish(MotionEvent event) {
        int index=event.getActionIndex();Capture c=pointers.get(event.getPointerId(index));
        if (c==null) return;
        position(c,event.getX(index),event.getY(index),sourceModifiers(event,true));
        pointers.remove(c.id);order.remove(c);
        completedTap=!c.dragged;
        try { c.target.up(c.x,c.y,c.modifiers,false); }
        finally { if (order.isEmpty()) surface.captureChanged(false); }
    }
    /** Call before replacing controls/transforms, losing the window or detaching.
     * Cancellation ends each edit once at its last delivered source coordinate.
     * A callback-requested cancellation is deferred until that callback returns. */
    public void cancel() {
        if (dispatching) { cancelPending=true;return; }
        cancelPending=false;ownedStream=false;
        ArrayList<Capture> active=new ArrayList<>(order);order.clear();pointers.clear();
        RuntimeException failure=null;dispatching=true;
        try {
            for (Capture c:active) if (c.begun) try { c.target.up(c.x,c.y,c.modifiers&~112,true); }
            catch (RuntimeException error) { if (failure==null) failure=error; }
            if (!active.isEmpty()) surface.captureChanged(false);
        } finally { dispatching=false;cancelPending=false; }
        if (failure!=null) throw failure;
    }
    public boolean onTouchEvent(MotionEvent event) {
        if (dispatching) throw new IllegalStateException("Recursive touch dispatch");
        completedTap=false;
        if (event.getActionMasked()==MotionEvent.ACTION_DOWN) cancel();
        boolean previouslyOwned=ownedStream;
        dispatching=true;
        try {
            switch (event.getActionMasked()) {
                case MotionEvent.ACTION_DOWN:
                case MotionEvent.ACTION_POINTER_DOWN: start(event);break;
                case MotionEvent.ACTION_MOVE:
                    for (int history=0;history<event.getHistorySize() && !cancelPending;++history) move(event,history);
                    if (!cancelPending) move(event,-1);break;
                case MotionEvent.ACTION_POINTER_UP: finish(event);break;
                case MotionEvent.ACTION_UP: finish(event);cancelPending=true;break;
                case MotionEvent.ACTION_CANCEL: cancelPending=true;break;
                default: break;
            }
            return previouslyOwned || ownedStream;
        } catch (RuntimeException error) { cancelPending=true;throw error; }
        finally { dispatching=false;if (cancelPending) cancel(); }
    }
    public int capturedPointers() { return order.size(); }
    public boolean completedTap() { return completedTap; }
}
