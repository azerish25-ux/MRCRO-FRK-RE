package com.mfrk.midihost;

import android.content.Context;
import android.media.midi.MidiDevice;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiManager;
import android.media.midi.MidiOutputPort;
import android.media.midi.MidiReceiver;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;

/** Serializes MIDI 1 input from currently visible devices onto one worker thread. */
public final class MidiInputs implements AutoCloseable {
    public interface Listener {
        // Bytes are owned by the listener and timestamps use System.nanoTime().
        void onBytes(int deviceId, int port, byte[] bytes, long timestamp);
        // Reset that port's decoder and release its active notes on close/flush.
        void onPortReset(int deviceId, int port);
        void onError(int deviceId, String message);
    }

    private static final class Entry {
        final int id;
        final ArrayList<MidiOutputPort> ports = new ArrayList<>();
        final ArrayList<Integer> numbers = new ArrayList<>();
        MidiDevice device;
        Entry(int id) { this.id = id; }
    }

    private final MidiManager manager;
    private final Listener listener;
    private final HandlerThread thread = new HandlerThread("MiniFreak MIDI input");
    private final Handler handler;
    private final Handler openHandler = new Handler(Looper.getMainLooper());
    private final AtomicBoolean closed = new AtomicBoolean();
    // Accessed only on handler, including open callbacks and delivery.
    private final Map<Integer, Entry> devices = new HashMap<>();
    private boolean registered;

    private final MidiManager.DeviceCallback callback = new MidiManager.DeviceCallback() {
        @Override public void onDeviceAdded(MidiDeviceInfo info) { open(info); }
        @Override public void onDeviceRemoved(MidiDeviceInfo info) { remove(info.getId()); }
    };

    public MidiInputs(Context context, Listener listener) {
        if (listener == null) throw new IllegalArgumentException("listener");
        this.listener = listener;
        manager = (MidiManager) context.getApplicationContext().getSystemService(Context.MIDI_SERVICE);
        thread.start();
        handler = new Handler(thread.getLooper());
    }

    public void start() {
        if (closed.get()) return;
        handler.post(() -> {
            if (closed.get() || registered) return;
            if (manager == null) { listener.onError(-1, "MIDI service unavailable"); return; }
            // A second enumeration after registration closes the discovery race.
            try {
                MidiDeviceInfo[] initial = manager.getDevices();
                manager.registerDeviceCallback(callback, handler);
                registered = true;
                for (MidiDeviceInfo info : initial) open(info);
                for (MidiDeviceInfo info : manager.getDevices()) open(info);
            } catch (SecurityException error) {
                listener.onError(-1, "MIDI service access denied");
            }
        });
    }

    private void open(MidiDeviceInfo info) {
        if (closed.get() || info.getOutputPortCount() == 0 || devices.containsKey(info.getId())) return;
        final Entry entry = new Entry(info.getId());
        devices.put(entry.id, entry);
        try {
            manager.openDevice(info, device -> {
                // Open completion may arrive after close() has stopped the worker.
                if (closed.get() || !handler.post(() -> finishOpen(info, entry, device))) closeDevice(device);
            }, openHandler);
        } catch (IllegalArgumentException | SecurityException error) {
            devices.remove(entry.id);
            listener.onError(entry.id, "MIDI device access denied");
        }
    }

    private void finishOpen(MidiDeviceInfo info, Entry entry, MidiDevice device) {
        if (closed.get() || devices.get(entry.id) != entry) { closeDevice(device); return; }
        if (device == null) { devices.remove(entry.id); listener.onError(entry.id, "Unable to open MIDI device"); return; }
        entry.device = device;
        for (int port = 0; port < info.getOutputPortCount(); ++port) {
            MidiOutputPort output;
            try { output = device.openOutputPort(port); }
            catch (IllegalArgumentException | SecurityException error) { listener.onError(entry.id, "Unable to open MIDI output port " + port); continue; }
            if (output == null) { listener.onError(entry.id, "Unable to open MIDI output port " + port); continue; }
            final int portNumber = port;
            output.connect(new MidiReceiver() {
                @Override public void onSend(byte[] bytes, int offset, int count, long timestamp) {
                    if (closed.get()) return;
                    byte[] copy = Arrays.copyOfRange(bytes, offset, offset + count);
                    handler.post(() -> { if (!closed.get() && devices.get(entry.id) == entry) listener.onBytes(entry.id, portNumber, copy, timestamp); });
                }
                @Override public void onFlush() {
                    handler.post(() -> { if (!closed.get() && devices.get(entry.id) == entry) listener.onPortReset(entry.id, portNumber); });
                }
            });
            entry.ports.add(output);
            entry.numbers.add(port);
        }
    }

    private void remove(int id) {
        Entry entry = devices.remove(id);
        if (entry == null) return;
        for (int i = 0; i < entry.ports.size(); ++i) {
            listener.onPortReset(id, entry.numbers.get(i));
            try { entry.ports.get(i).close(); } catch (IOException ignored) { }
        }
        closeDevice(entry.device);
    }

    private static void closeDevice(MidiDevice device) {
        if (device != null) try { device.close(); } catch (IOException ignored) { }
    }

    @Override public void close() {
        if (!closed.compareAndSet(false, true)) return;
        handler.post(() -> {
            if (registered) manager.unregisterDeviceCallback(callback);
            for (int id : new ArrayList<>(devices.keySet())) remove(id);
            thread.quitSafely();
        });
    }
}
