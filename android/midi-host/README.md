# Android MIDI input foundation

`MidiInputs` discovers currently visible Android MIDI 1 byte-stream devices,
opens their output ports, follows hot-plug changes, and serializes copied input
bytes and timestamps onto one worker. The consumer must keep a separate native
stream decoder per device and port, handle port resets, and enqueue events for
the audio thread without blocking it.

This is a library, not the standalone instrument. It is not yet connected to the
native MIDI decoder, note/controller graph, audio callback, or user interface.
Android exposes Bluetooth MIDI devices only after a separate Bluetooth open; MIDI
2 universal packets, Bluetooth discovery/permission handling, reconnection policy,
and real-device behavior remain to be completed. The remote workflow builds the
library; it does not prove behavior with physical MIDI hardware.
