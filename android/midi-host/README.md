# Android MIDI input foundation

`MidiInputs` discovers currently visible Android MIDI 1 byte-stream devices,
opens their output ports, follows hot-plug changes, and serializes copied input
bytes and timestamps onto one worker. `NativeMidiBridge` connects that worker
to the per-port MIDI decoder and bounded single-producer/single-consumer queue.
The future audio host must check overflow before draining, reset notes and
controllers when needed, and detach its native pointer before bridge close.

This is a library, not the standalone instrument. It is not yet connected to the
note/controller graph, audio callback, or user interface.
Android exposes Bluetooth MIDI devices only after a separate Bluetooth open; MIDI
2 universal packets, Bluetooth discovery/permission handling, reconnection policy,
and real-device behavior remain to be completed. The remote workflow builds the
library; it does not prove behavior with physical MIDI hardware.
