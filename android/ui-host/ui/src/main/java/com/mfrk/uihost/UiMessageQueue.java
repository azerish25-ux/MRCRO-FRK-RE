package com.mfrk.uihost;

import android.os.Handler;
import android.os.Looper;

/** Process main-loop transport for native owned controller messages. */
public final class UiMessageQueue {
    private static final Handler MAIN = new Handler(Looper.getMainLooper());
    static { System.loadLibrary("mfrk_ui_messages"); }
    private UiMessageQueue() {}

    /** Initialize on the UI thread before constructing a native scene/backend. */
    public static void initialize() {
        if (Looper.myLooper() != Looper.getMainLooper())
            throw new IllegalStateException("UI message initialization requires the main thread");
        nativeInitialize();
    }

    private static boolean enqueue(long handle) {
        return MAIN.post(() -> nativeDeliver(handle));
    }
    private static native void nativeInitialize();
    private static native void nativeDeliver(long handle);
}
