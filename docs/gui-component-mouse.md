# Component hover event delivery

`ui_component_mouse.hpp` implements source `ff78a0/ff7a90/ff8570` enter, exit
and move delivery and `108e330` listener propagation. It preserves event source
identity, rounded local integer positions, current nonmouse modifier merging,
mouse-down position/time defaults and zero pen/click/drag fields. Enter sets the
hover bit after the direct callback; exit clears it beforehand. Optional hover
repaint precedes those operations. Modal blocking routes to cursor hiding or
Desktop movement notification.

After the direct callback, live targets notify reverse-order Desktop listeners,
local listeners and each ancestor's deep-listener prefix. Every source weak-token
check and live count adjustment is retained, including independent source and
ancestor liveness checks. Reparenting a living target changes later propagation.

The original executable comparison matches all 288 cases and 2,250 service events
locally. It covers modal gates, hover bits, half-point rounding, listener removal,
target/ancestor retirement, reparenting and callback-driven modifier changes.
`core/tests/component_mouse_provenance.json` binds exact evidence identities.
The queued Android pointer probe now passes enter/exit/move through this body.

The admission uses an existing Desktop owner and weak tokens, complete acyclic
ancestors, signed32 listener counts, valid deep prefixes, surviving registration
objects and retained component storage through enter's callback/flag write.
Modal policy, repaint, Desktop timer/blocked notifications, cursor and actual
widget callbacks remain explicit services. Full scene/touch/widget/controller
integration remains unfinished.

Source `ff7c80/ff8330/ff8050/ffb750` down, drag, release and click history are now
implemented too. This retains modal-attempt/unblocking behavior, ancestor
stacking/focus order, press-coordinate rebasing, pre-release modifiers, pen data,
long-press movement status and the separate double-click callback/listener pass.
The original comparison matches all 384 cases and 2,351 events locally, including
time/distance/touch thresholds, packed peer/modifier comparisons and callback
mutations. `component_mouse_buttons_provenance.json` records identities.
Signed64 times, their differences and the source 300ms addition must fit; actual
coordinate service admission also applies. `GuiMouseComponentBinding` now joins
the real pointer state owner to geometry and all six component event bodies.
