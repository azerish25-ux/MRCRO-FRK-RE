# GUI message delivery

`GuiMessageEnvelope` owns a queued message and a weak listener reference. Delivery
consumes the message once. An expired listener receives no callback; the owned
payload is still cleaned up. A live listener is retained through its callback.

`GuiControllerMessageListener` checks the weak backend owner before testing the
message family. An expired owner or another message family is ignored. Type 0
calls the actual owned writer, retaining the original origin, ID and mutable
typed value. `GuiOwnedControllerRecipient` connects that call to the recovered
outer write owner, router and parameter policy. Delivery never bypasses the
outer owner. If invoked on another thread, the real router can enqueue again.

Valid kinds 1..6 require `GuiOtherControllerMessages`; there are no successful
empty preset/control handlers. Unknown kinds are ignored as in the original
listener. The actual preset/default-preset and other handler bodies remain
application services to implement. This is not a completed general UI backend.

The native comparison executes `f7b750`, `667b10`, and the full type-0 write chain
with actual receiver/cache code. Compiler RTTI and the existing write endpoint
services are supplied. Other kinds 1/2/3/4/6 terminate at explicit observers;
kind 5's default-preset body is not covered by this proof. All 356 cases,
111 policy events, 252 outer events and 10 other-handler events match locally.
Coverage includes missing tokens/listeners/owners, wrong message family,
unknown kinds, valid/invalid parts, positive/internal receivers and normal or
wrong-thread delivery. The fixture is hash bound and configured for Android CI.

Callbacks require stable service lifetimes and serialized owner access.
Applications can alias recipient/listener shared pointers to their scene owner
so a callback retains the whole scene and its services. The platform queue must
own envelopes until delivery or cancellation. Android Handler/JNI scheduling,
actual scene binding, all message-handler bodies and full app parity remain
unfinished.
