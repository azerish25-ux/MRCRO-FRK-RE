# Reference exponential mappings

The source curve factory now recognizes `ExponentialPotardMapping`, preserving
the original forward `(expf(value*k)-1)*scale` and inverse
`logf(value/scale+1)/k` binary32 operation order (`59d900` / `59d930`). It accepts
positive finite `k<88`, positive finite scale, normalized inputs 0..1, and a
finite inverse logarithm argument. Unsupported fields are rejected.

The live descriptor census contains 19 instances with three exact field pairs:

| k bits | scale bits | Uses |
|---|---|---|
| 40c00000 | 3b22d9e6 | Delay low-pass and EQ cutoff controls |
| 40a00000 | 3bde496c | EQ bandwidth controls |
| 411c0000 | 38748512 | Multiband compressor ratios |

Their exact parameter names, descriptor indexes, display ranges and fields are recorded in
`resources/gui-mapping-census.json`. The factory now covers the mapping classes
and field profiles observed in that descriptor-default census. Widget custom
GUI info and display mappings remain separate; this is not full UI coverage.

The math reconstructs the pinned UCRT 10.0.26100.9711 FMA-dispatch `expf` and
positive `logf` value paths. Exact tables, coefficient bits and fused operation
order are preserved. The exponential polynomial differs from the one used by
the master-volume power helper. The logarithm uses binary32 fused operations;
an exact double product, TwoSum residue and midpoint correction reproduce their
single rounding without relying on a platform float-FMA fallback. No platform
`expf` or `logf` approximation is used.

The admitted helper domains are exponential inputs 0..<88 and finite logarithm
inputs >=1, with nearest-even rounding, gradual underflow and disabled implicit
FP contraction. Value bits are modeled; errno and floating exception flags are
not. The 20,874 original checked results cover complete forward/inverse bodies
for all three live field pairs, direct runtime helper results, near-one and
table boundaries, subnormals, signed zero, float-FMA fallback failures and
random interiors. All comparisons match locally.

These mappings are shared native control code. Application parameter hosts,
registration lifetime, full scene rendering and the standalone sound/MIDI graph
still need integration.
