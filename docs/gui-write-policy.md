# GUI write policy

`gui_apply_write` reconstructs backend `678190` control flow with actual
descriptor, receiver, transaction, association, selector and host services.
It retains begin/end ordering, origin exclusions, type/size admission,
mode-specific early delivery, automation state, receiver modes, transformed
writes, feature-reset writes, association effects, forced contexts, part gates,
replacement values, host edit suppression and processor delivery.

`GuiInternalWriteReceiver` and `GuiPositiveWriteReceiver` bind it to the existing
owned parameter implementations. Ordinary writes run the recovered typed move,
dirty state, notification and positive normalized-cache/FObject update.
Origin 6 follows `6d1ac0`: broadcast the incoming value without storing or
consuming it, then inspect the receiver's existing stored value for subsequent
policy work. The broadcast keeps the incoming value shared by reference.
Both original vtables use that direct method: InternalParam `181be22c0` and
VstParam `181bc6c08`.

The original-instruction comparison executes the whole native policy, both
receiver types, getter/setter/direct broadcast, scalar equality, typed move,
positive cache and FObject update with a null UpdateHandler. Native `__chkstk`
executes against the fixture's owned TEB and committed stack; it is not bypassed.
Descriptor/prewrite, transaction, transformation/association, terminal
notification, selector, part save, host/processor and begin/end endpoints are
explicit observers in this comparison.

All 2,160 finite scalar cases, 14,011 events, return bytes, values, dirty/cache
state and automation state match locally. The cases cover positive/internal
IDs across three parts, absent receivers, origins 0/1/2/3/4/5/6/8/9/10/12/19,
descriptor flag combinations, transaction gates, context binding scans,
association selection, part replacement and reset sequences. The immutable
fixture is hash checked and included in Android CI.

This proof uses valid scalar descriptor profiles and empty transformed-value
lists. Typed data, malformed descriptors, nonempty transform lists, service-time
state mutation and nonempty host UpdateHandler behavior need further integrated
comparison. The generic control flow exposes those services rather than
installing default successes. The outer `817fb0` hardware wrapper, queue delivery,
concrete scene service implementations, full touch/visual parity, DSP/FX and
standalone application remain unfinished.
