# Reference master-volume mapping

The curve factory now recognizes the original
`BipolarVariableCenterInvPowPotardMapping` for the live MasterVolume descriptor's
exponent 2 and center 0.5. It executes the original forward/inverse branch order
and binary32 range arithmetic (`59e550` / `59e5e0`). Other exponent/center fields
are rejected. The admitted control input range is 0..1, including signed zero
and subnormals. The descriptor's display range is -70 to +6.

The source calls Windows UCRT `powf`; replacing it with square/square root
changes results. The port reconstructs the observed runtime's positive FMA
value path using exact binary64 constants, reciprocal/log/exponential tables,
explicit fused operations, and the original source underflow threshold. Its
near-one branch deliberately keeps the source's unfused logarithm and separate
unfused exponential polynomial. It does not call the Android platform's `powf`.

The pinned reference runtime is 10.0.26100.9711, SHA-256
`dedb7611e1abc1ff3852dcac3d4c93a048d729e25b061973b9969f22698f14ca`.
Read-only observation of the running reference confirmed its FMA dispatch flag
was 3. This implementation reproduces value bits for that observed profile;
errno, floating exception flags, alternate runtime dispatch modes and other
exponents are outside this helper's contract. Compilation requires nearest-even
rounding, gradual underflow, and disabled implicit FP contraction. Explicit
`std::fma(double)` calls retain the source's single-rounding operations.

The 14,836 checked original results cover the complete forward/inverse bodies
and the runtime helper: signed zero, subnormals, table boundaries, center and
near-one neighborhoods, underflow neighborhoods, failed-shortcut examples and
random interiors. The local comparison reports zero differences. An earlier
private prototype also matched two million direct runtime comparisons. The
fixture contains synthetic inputs/results, not a vendor binary or preset dump.

This adds the actual master-volume mapping to the shared native curve factory.
The full UI parameter host, control registration, preset delivery through the
complete sound graph and standalone Android instrument remain separate work.
