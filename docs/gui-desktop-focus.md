# Desktop focus notifications

`GuiDesktopFocusUpdates` supplies the original `ffd3d0` callback through the
retained Android update channel. It captures the focused component once using
its weak token, then traverses registered listeners in reverse order with live
size repair. A changed global focus does not replace that dispatch's captured
component. If the component expires, later listeners receive null. Re-triggered
delivery captures the then-current focus in the next main-loop message.

The listener registry is an actual mutable ordered list supplied by its owner.
Appending a listener does not extend the current traversal. Removing an earlier
entry may notify the current listener again after index repair, retaining the
source behavior. `GuiOwnedComponentFocusBoundary` joins concrete component focus
operations, real posting and this listener callback; application peer/input/
accessibility methods and registered listeners remain required implementations.

The comparison executes original Desktop code, weak guards and the underlying
async trigger/post/callback paths. All112cases/200notifications match locally,
including redirection, expiration, append/removal/clear and re-triggered delivery.
The driver models dequeue/ref release and retains expired node storage. Registry
and Desktop owner survive dispatch; focused-node and listener lifetimes may
change through valid callbacks. Full component destruction remains a separate
application responsibility.

Six remote Android cases additionally check reverse order, actual focused-node
deletion, redirected snapshots, earlier-listener removal, clear and append with
retrigger. They use the production Desktop target and Handler/JNI transport;
listener callbacks are explicit debug observers. Remote validation is pending.
The full scene and original raster/audio/MIDI/preset integration remain unfinished.
