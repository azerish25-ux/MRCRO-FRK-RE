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
