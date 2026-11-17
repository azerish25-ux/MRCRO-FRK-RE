# GUI write ownership

`gui_write_owned` joins outer wrapper `817fb0` to the recovered write router.
It retains the special-parameter integer cache, mutex gate, hardware eligibility,
typed copy and live recursion guard. A concrete `GuiWriteMutex` supplies platform
locking; busy returns false and mutex errors propagate. Eligibility runs after
unlocking. The special cache uses source signed-32 truncation, including the
indefinite integer for NaN, infinity and out-of-range inputs; FP status is outside
the contract.

The preserved value is copied after eligibility and before routing, because the
router may move the input into a queued message. Hardware forwarding uses that
copy even when routing rejects an invalid part. It runs only when this call set
the guard and the live guard is still set. Nested eligibility/send writes follow
the same real wrapper and router; existing guards suppress recursive forwarding.

The original-instruction comparison executes `817fb0`, `678050`, `64ea00`,
`6682f0`, `678190`, concrete receiver/cache code and typed copy/move. It compares
the composed portable wrapper/router/policy with actual owned parameter
receivers. Mutex, hardware eligibility/send, queue and part-bank interfaces are
observer services; the policy endpoint scope is documented separately.

All 474 cases, 1,627 outer events and 806 policy events match locally. Coverage
includes ordinary/internal IDs, valid/invalid parts, busy locks, existing/live
guard changes, eligibility mutation before copying, nested eligibility/send
writes, queued inline/heap data and special-cache conversion edges. The fixture
checks complete queued/input payload hashes, receiver/cache state and traces.

This is the write-owner composition, not the complete Android backend. Actual
hardware transport, queue delivery/recipient lifetime, descriptor/association
services, scene binding and the standalone audio/MIDI application remain open.
