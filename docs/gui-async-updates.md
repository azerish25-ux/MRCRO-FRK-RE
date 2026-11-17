# Owned asynchronous updates

`GuiAsyncUpdater` follows the original `f7c7c0` message callback and `f7c8a0`
owner destruction. A message owns its atomic pending word and borrows its target.
Successful queue entries retain the message. Delivery changes pending1 to0
before invoking the target; cancellation/destruction clears pending before
dropping the owner's message reference. Queued cancelled messages can therefore
outlive the target safely. A callback can retrigger or destroy its owner while
the current queue entry retains the message.

Triggering preserves the original atomic0-to1 admission and failed-post reset.
Cancellation followed by retriggering may leave multiple queue entries for the
same message; the first eligible callback consumes its current pending state.
The mouse helper retains `ff8b50` ignore/child-click filtering and dragging-mask
checks using actual source mouse-state fields supplied by the input owner.

`GuiComponentFocusUpdate` binds the component focus owner's pending flag to that
same message word. It supplies the focus boundary's actual post operation and
restores the unbound logical flag after cancellation/teardown. A replacement
channel uses a fresh message word; old cancelled entries cannot consume the new
owner's pending update. The state and target outlive their channel, and producers
stop using an updater before destroying it. Target callbacks/destruction are
serialized on the UI thread.

The native comparison executes the original mouse trigger, native queue post
and reference accounting, callback and updater/message destructors. All64cases,
547steps and67callbacks match locally, including message retention after owner
destruction, cancel/retrigger, callback-time retrigger/destruction, post failure
and mouse filters. The fixture driver models dequeue/ref release and explicit
cancellation, with a reserved16-entry native queue. Platform lock/wake/free
services are modeled endpoints; this is not a full MessageManager proof.

`android/ui-host/native/ui_async_update_queue.cpp` posts these retained messages
through the existing real Handler/JNI transport. Its process dispatcher retains
no Activity, scene, updater or target. Debug instrumentation adds eight real
main-loop cases: coalescing, cancellation, deleted owner, nested updates,
cancel/retrigger, replacement owner, self-deletion and worker coalescing. These
checks passed in [renderer run44050389250](https://github.com/azerish25-ux/MRCRO-FRK-RE/actions/runs/44050389250),
together with the six existing queue cases and five focused comparison frames.
Native run44050389269 also passed the original-code fixture on Android.
Exact source/artifact identities are in `validation/async-android-20261117.json`.
The [Desktop focus notification target](gui-desktop-focus.md) now supplies actual
registered-listener dispatch. Actual mouse callbacks, peer/input services and
the complete scene still need their application implementations.
