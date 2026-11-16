# Owned parameter notification

`ParameterNotifier` connects the recovered listener-side ownership to a concrete
forward registry. It owns three primary and three alternate part maps, preserves
the native pointer-byte FNV/list ordering and bucket growth of each listener
set, and routes reverse-owner cleanup through `NotifierDirectory`. The directory
must outlive its notifiers and listeners; unknown notifier identities are
ownership errors, not silently ignored registrations.

Registration inserts the reverse association first. A duplicate parameter ID
keeps its original notifier and does not create another forward membership,
including attempts to register it in an alternate group or another notifier.
Removal searches primary before alternate, removes the set member, then clears
the reverse association unless the listener is already being destroyed. Empty
forward entries are removed.

Primary notification executes the recovered dispatcher with a cloned listener
snapshot and shared payload. Application callbacks are supplied explicitly and
must remain valid for that notification even if membership is removed. Closing
the notifier forgets its reverse associations without callbacks, clears primary
then alternate for each part, and marks the notifier destroying afterwards.
Listener cleanup has the opposite guard timing: it marks itself destroying
before removing forward subscriptions. Both cleanup directions are integrated.

The original-instruction comparison executes real native forward lookup,
pointer-set insertion/removal/clone, reverse-map operations and cleanup from a
supplied empty layout validated against the live editor. Raw allocation and
listener callbacks are explicit services. Across 600 steps and 3,141 callbacks,
all local state and trace comparisons match, including 520 listeners, all three
parts, alternate membership, duplicate cross-notifier registration, removal and
nested missing notification during callbacks, scalar mutation and notifier
closure during an active cloned notification.

Forward keys use portable storage and are canonicalized by key in the state
comparison. The physical outer-table slot layout is not an application API;
member order, reverse order and complete callback fields remain exact. No
upstream container-version identity is inferred from layout similarity.

This is the concrete ownership/primary notification component, not the full GUI
host. Actual Arturia callback families, backend edit/write policies, registration
feature-ID mapping, bank transfer, scene services and standalone synth assembly
remain separate work. No scalar-store or empty application callback fallback is
installed.
