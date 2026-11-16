# Listener registration ownership

`ListenerRegistry` reconstructs the source IParamListener's reverse map from
parameter ID to notifier. It owns the registrations, keeps the native four-byte
FNV hash/list order, preserves the first notifier on duplicate insertion, and
reproduces bucket growth and rehash order. Lookup and removal use a separate
index whose iteration order does not affect the native registration order.

The supplied notifier-removal service must outlive this owner. A notifier must
remain valid until its registrations are removed or explicitly forgotten. On
listener cleanup, the registry sets its destroying flag before issuing removal
calls in source order, skips null notifier entries, and keeps the reverse map
stable until callbacks finish. Guarded reverse removal during this phase is a
no-op, as in the original. Cleanup releases the owned storage afterwards; the
portable `close()` API is idempotent and is also called by the destructor.
Insertion during cleanup is outside the admitted profile and is rejected.

The original-instruction comparison covers four cases, 1,187 insert/duplicate
operations, 19 removals and 1,044 cleanup callbacks. Sequential, shuffled and
colliding IDs cross the 8, 64, 512 and 1,024 bucket states. The comparison checks
registration order, lookup values, duplicate behavior, removal results, and the
destroying flag and stable size seen by every cleanup callback. All comparisons
match locally.

The native capture executes the original constructor, insertion, rehash, lookup,
guarded reverse erasure and destructor. It also executes the original aligned
allocation wrapper so large-vector headers and deallocation guards remain
intact. Raw allocation/free and the outer notifier's lookup/set operations are
explicit observer services. No synth code or runtime guard was patched.

`forget_notifier` supplies the complementary source path (`64f920`): when a
notifier is being cleared, remove every reverse association with that notifier
without issuing callbacks into it. Shared notifier values, null entries,
missing/repeated removal and the listener's destroying guard match the original.
The separate comparison covers 593 registrations, 16 forget operations and
410 remaining cleanup callbacks, with zero differences locally. This enables
the notifier owner to sever its references before freeing itself.

This supplies listener-side ownership for the GUI host. The notifier's forward
registry, application registration policies, bank transfer and full UI host
integration remain separate work.
