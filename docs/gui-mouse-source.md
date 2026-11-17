# Pointer source ownership

`GuiMouseInputSource` follows original `ffab60/ffa920/ffa580/ffb150/ffb280`.
Its state owns the current position/offset, button state, weak target slots,
event serial, five pen-data words and four complete32-byte press records.
Target changes retain original release/exit/enter/cursor ordering, including
the new weak target visible during the previous target's exit callback.
Expired targets are rechecked at the source call points.

Movement retains sentinel-position behavior, forced refreshes, the original
four-pixel drag threshold and captured pen data. Button transitions preserve
mouse-up modifier merging, event-serial early returns, byte-exact history shifts,
peer IDs, touch-type byte and untouched press-record padding. Unbounded mode
retains screen-edge tests, offset accumulation, cursor restoration and source
conversion ordering; platform cursor/coordinate functions remain required.

`GuiMouseSourceUpdates` supplies the actual `ffb0d0` queued target over the shared
Android async queue. It uses the maximum of current epoch time and the stored
last-event time, then forces a refresh through the same state owner. Its pending
flag is the actual retained message word, with cancellation/teardown semantics
shared with the previously verified async channel.

The comparison executes all six original entry bodies and weak-reference
accounting. All384cases/1370events match locally, including callbacks changing
modifiers/serial/offset/pending, weak expiration, drag history and cursor paths.
The fixture supplies pre-existing weak tokens for both nodes, matching its
native setup; it retains expired node storage and does not prove full native
component destruction. Coordinates/intermediates are finite and bounded, button
state uses the source mask, access is serial, and component storage survives
coordinate/cursor service calls.

Hit resolution, local/global/display transforms, component event bodies, peer,
clock and cursor services are observed endpoints and mandatory application
implementations. The scene factory and real Android touch/hit bindings remain
unfinished. Six queued Android checks cover forced refresh, both sides of the
drag threshold, actual target deletion, target transition and clock regression;
remote validation is pending. Full scene/raster/audio/MIDI/preset parity remains
the active objective.
