# GUI controller registration

`GuiRegisteredParameter` owns the reverse registry, recovered controller methods
and notification handler as one object. Its constructor follows `6612b0`:
register the wrapped parameter, accept the backend-allocated origin, then
subscribe to the descriptor-selected feature/property events in source order.
Its cleanup follows `6614c0`, rereading the live descriptor flags, unregistering
the wrapped parameter, then removing any remaining reverse associations.

`GuiNotifierRegistration` connects those operations to the concrete notifier.
Selector IDs come from `GuiIdTables`, preserving `67d450` / `699f00` bounds,
context-table selection, signed ID bits and invalid sentinel behavior. Invalid
selectors are optional, not parameter zero. Feature/property registration uses
the current context, prepares the part and requires a real receiver before
adding the subscription. Own-parameter registration uses the primary context
and increments the supplied origin counter even for the invalid own-ID sentinel.
No fixed GUI origin or successful-receiver fallback is supplied.

The original-instruction comparison executes the actual constructor/destructor,
both native selector lookup routines and real forward/reverse notifier code.
Backend lifecycle calls are explicit services that delegate to that native
notifier. The portable comparison uses initialized real preset banks for
receiver checks. All 192 cases and 2,139 lifecycle events match locally,
including missing selectors, negative internal IDs, changed context/descriptor
state, subscription-time flag changes, counter wrap and final cleanup.

The live selector snapshots are in `resources/gui-selector-tables.json`.
Context value 1 selects the second table; other values select the first.
Feature lookup has 25 categories per part; property lookup has 68. The snapshots
contain IDs only, not parameter values or live pointers. Applications must
supply actual current tables and part/bank services as their state changes.

This joins recovered registration, controller and callback ownership. Actual
backend write/edit policy, association services, scene/view binding, timers,
touch coordinate calibration, full sound graph and standalone assembly remain
unfinished.

`gui_dispatch_write` preserves backend `678050` routing: when a message thread
exists and differs from the caller, it moves the typed value into an owned queue
message before looking up a part or receiver. With no message thread, or on that
thread, it validates the sign-masked million-ID part against the actual part
count, prepares it, resolves its receiver and calls the actual write policy.
A null receiver is passed through to that policy. No scalar assignment or
successful-receiver substitute is introduced. The explicit route enum is a
portable diagnostic; the native synchronous return register is not treated as
a success flag.

The original `678050`, `64ea00` and typed move `64b2d0` match 216 local cases and
432 events, including scalar/inline/heap values, absent/same/other message
threads, negative IDs, invalid parts, null receivers and policy consumption.
Queue/thread/part preparation/receiver/full write policy are observer services
in that proof. The Android queue consumer, outer `817fb0` wrapper and complete
`678190` policy remain unfinished; this is not an installed app write backend.
