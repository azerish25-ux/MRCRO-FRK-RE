# Component focus ownership

`GuiComponentFocusBoundary` supplies the visibility boundary's concrete focus
operations through one shared `GuiComponentFocusState`. Source `ff92b0/ff90d0/
ff9450/ff8e60/ff8f30/ff9780/ff2ed0` determines ancestor enablement, peer lookup,
default-component traversal, parent fallback, peer focus requests and old/new
focus notifications. The focused pointer changes before the previous component
is notified. Subsequent callbacks observe the current pointer and actual weak
token, including focus redirected during a callback.

Component `focus_flags` projects the original byte at `+d1`; child-keyboard-focus
and disabled bits share that byte. Child-focus changes propagate through current
parents with per-component weak guards. Root focus requests preserve the original
enablement exception. Traversers are real owning results: default-component
recursion precedes their destruction, which precedes parent fallback. Returning
an ineligible default component does not invent a second fallback attempt.

The Desktop focus-update flag coalesces posts with the original atomic 0-to-1
transition. A failed post resets it. The [owned asynchronous update channel](gui-async-updates.md)
now supplies actual message retention, delivery/cancellation and Android posting.
Desktop focus-listener dispatch remains an application responsibility.
Accessibility loss, gain requests and descendant
focus checks use the same shared accessibility-focus identity as the source.

The native comparison executes these original focus bodies together with widget
visibility, repaint, weak guards and Desktop pending-flag atomics. All 768 cases
and 1,750 service events match locally. It checks callback-time redirects,
expired tokens, peer-already-focused handling, default traversal/disposal,
notification suppression, post failure/coalescing and ancestor state. Joined
knob visibility uses this concrete owner, rather than the earlier observed focus
endpoints. The fixture authenticates complete bytes before interpreting input.

Real peer methods, traversal policy, focus callbacks, accessibility policy and
queue posting are observed endpoints. Their Android implementations and actual
scene construction remain required. Lifetime-expiration cases retain fixture
storage; final destructor behavior is not established. Access to scene state is
serial, parent/default traversal is acyclic, and every focus participant/ancestor
is a complete `GuiVisibilityComponent` owned by the scene. The separate parent
mapping and image-release profiles from `gui-slider-view.md` still apply.
