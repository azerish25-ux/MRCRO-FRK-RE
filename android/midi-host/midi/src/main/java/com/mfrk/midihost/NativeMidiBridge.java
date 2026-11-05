package com.mfrk.midihost;

/** Connects MidiInputs' one worker to the native MIDI decoder and SPSC queue. */
public final class NativeMidiBridge implements MidiInputs.Listener, AutoCloseable {
    public interface Errors { void onError(int deviceId, String message); }
    static { System.loadLibrary("mfrk_midi_jni"); }
    private final Errors errors;
    private long handle;

    public NativeMidiBridge(Errors errors) {
        this.errors = errors;
        handle = nativeCreate();
        if (handle == 0) throw new OutOfMemoryError("MIDI ingress");
    }

    // The audio host must stop consuming and detach this pointer before close().
    // It points to mfrk::midi::Ingress; its consumer calls recover_overflow()
    // before each drain, then resets notes/controllers when recovery is needed.
    public synchronized long nativeHandle() { return handle; }

    @Override public synchronized void onBytes(int deviceId, int port, byte[] bytes, long timestamp) {
        if (handle != 0 && !nativeFeed(handle, deviceId, port, bytes, timestamp) && errors != null)
            errors.onError(deviceId, "MIDI input overflow or source limit");
    }
    @Override public synchronized void onPortReset(int deviceId, int port) {
        if (handle != 0 && !nativeResetPort(handle, deviceId, port) && errors != null)
            errors.onError(deviceId, "MIDI input overflow");
    }
    @Override public void onError(int deviceId, String message) { if (errors != null) errors.onError(deviceId, message); }
    @Override public synchronized void close() { if (handle != 0) { nativeDestroy(handle); handle = 0; } }

    private static native long nativeCreate();
    private static native void nativeDestroy(long handle);
    private static native boolean nativeFeed(long handle, int device, int port, byte[] data, long timestamp);
    private static native boolean nativeResetPort(long handle, int device, int port);
}
