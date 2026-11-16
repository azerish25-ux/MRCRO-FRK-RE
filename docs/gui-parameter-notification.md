# GUI parameter notification

`GuiParameterNotification` reconstructs the source primary callback `6616b0`
and its changed/scalar deliveries `661640` / `661660`. It shares the live
controller state with the existing read/write/edit methods. The original editor
has non-null control-info state on the observed filter control, so the feature
prefix is implemented instead of bypassed.

The callback queries the same property and feature services in source order,
updates the control-info configuration/state fields, preserves invalidation
requests, and handles the valid eight-byte macro and 24-byte MIDI-assignment
messages. Association lookup and selection policies remain explicit application
services; no default selected ID or false eligibility is substituted. Raw field
names retain source offsets where wider UI meaning remains to be established.

Origin 5 propagates a linked parameter's typed payload to enabled alias rows
using a separate owning copy for each backend write. Alias count/enabled state,
backend, observer and control-info pointers remain live across callbacks.
Ordinary own-parameter notification is suppressed for the controller's allocated
origin. Origin 6 uses raw scalar delivery only for tag-1 values; other origins
use changed delivery. Feature/data messages preserve their source priority and
do not collapse into a generic parameter redraw.

All 640 original-instruction cases and 4,249 service events match locally. The
comparison exercises non-null control info, configuration flags, missing property
or feature IDs, association cases, linked aliases, typed data, direct scalar
delivery, origin suppression, invalidation-time ID/control-info replacement,
and alias-list growth/value consumption during backend writes. The original
callback, delivery and typed-copy routines execute; property/feature/association,
invalidation, observer and backend write calls are explicit services. Feature
state scalars are finite and data message lengths are valid in this profile.

The concrete scene invalidation, association services, registration construction
and complete application callback integration are still separate work. This
adds the callback behavior needed by that host, not a finished standalone UI.
