# Source GUI parameter delivery

`GuiParameterController` reconstructs the Juced controller's begin (`662900`),
end (`662940`) and scalar write (`662980`) methods. Its backend and observer are
application-owned. The registered origin, parameter ID, context and observer
remain live across callbacks; the second notification reads them again. A
temporary typed value is destroyed before change notifications, including when
the backend replaces it with heap data. No fixed origin or scalar-store fallback
is installed. The admitted profile has diagnostic logging disabled.

The 576 original-instruction cases include both notification flags, null and
changed observers, backend/ID/context replacement during callbacks, raw float
bits and replacement heap cleanup. Local C++ comparison reports zero differences.
The checked fixture and its provenance accompany the implementation. This does
not establish backend write/undo behavior, registration lifetime, edit-mode
selection or a complete Android parameter factory.

## Live filter configuration

The original standalone editor was restored and observed on 2026-11-16. The
read-only descriptor capture agrees at registry, actual controller construction
and actual GUI-info access. `resources/filter-slider-bindings.json` records the
semantic fields without preset values, raw assets or runtime object addresses.

| Parameter | Wrapped ID | Mapping range | Normal sensitivity | Fine sensitivity |
|---|---:|---|---:|---:|
| Vcf_Cutoff | 13 | 0 to 1, linear | 0.004 | 0.00025 |
| Vcf_Resonance | 14 | 0 to 1, linear | 0.004 | 0.00025 |
| Vcf_EnvAmount | 15 | -1 to 1, linear | 0.004 | 0.00025 |

The original widget geometry is 100 by 100 in its own coordinate space. Both
precision fields are -1, item count is zero, and local custom GUI info is off.
Actual cutoff pointer-begin callbacks confirmed the normal and fine sensitivity
bits with source modifiers 16 and 32 respectively. A finger supplies the source
primary-button semantics through `SourceTouchRouter`; fine mode still requires
an explicit touch interaction design consistent with the source behavior.

The live backend vtable resolves to `181bc0298`; the write method is `817fb0`,
not a bare parameter receiver. Begin/end also own edit bookkeeping and timestamps.
The comparison screen still requires an application host to attach real controls.
Full scene rendering, timers, preset recall through the full sound graph and the
standalone Android instrument remain unfinished.
