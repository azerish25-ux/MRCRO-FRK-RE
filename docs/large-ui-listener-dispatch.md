# Live UI listener fanout

Read-only inspection of the original Advanced editor found 4,958 primary
notifier keys and 11,036 listener memberships. The largest set contained 499
listeners in 512 buckets. The earlier supplied-state dispatcher adapter's
128-listener admission would reject that real UI set. The observed size
histogram is recorded in `resources/notifier-listener-census.json`; it contains
no parameter values or live listener pointers.

The dispatcher snapshot now admits up to 16,384 unique nonzero listener
identities, with power-of-two bucket counts from 8 through 65,536 and source
load factor 1. It preserves the source pointer-byte FNV hash and list insertion
order. An index-linked list reproduces native insertion before the first node
in an occupied bucket without repeatedly moving or hashing the entire list.
Duplicate validation uses a sorted copy before the callback snapshot is built.
Current source iteration and capacity remain supplied by the owning notifier.

The additional original-instruction comparison covers 44 profiles and 136,472
callbacks, including sizes 229, 492, 499, 512, 513 and up to 16,384 listeners.
It checks complete callback identity/key/origin/busy/payload traces by digest,
lookup activity, original-membership clearing during the first callback,
nested missing dispatch, shared-value mutation and final state. The original
aligned allocator and large-vector deallocation guards execute unchanged;
only raw allocation/free, exact-key lookup and callback services are observers.
The original 134 smaller profiles remain unchanged and still match.

This closes an observed admission gap before full host integration. The
notifier's forward ownership and lookup implementation, actual callback
families, bank transfer, complete scene and standalone synth remain unfinished.
