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
not establish backend write/undo behavior, registration lifetime, application
selection policies or a complete Android parameter factory.

The source edit gate (`662ce0`) now queries mode parameter 11, handles a linked
control, forwards both selection flags and context, and checks the live owner's
selection capability and optional selected ID. Assembly confirms that the
linked-control branch also forwards the flags/context omitted in the decompiler
display. Descriptor and controller fields are read again after callbacks.
All 384 original-body edit comparisons match locally, including callback changes
to backend, ID, link and descriptor. A live observer is required; the actual
application's mode/selection services and registration remain backend-owned.

The read side (`6620e0` / `64b7e0`) resolves the parameter once and returns its
stored scalar bits, including signed zero and nonfinite values. A null or data
value returns +0 with diagnostics disabled. It does not use the positive
receiver's separately clamped normalized cache. All 224 original-instruction
read comparisons match locally, including a backend/ID change during lookup.

`GuiBankParameterView` connects that accessor to actual preset-bank storage.
The backend supplies the selected part's bank and its local wrapped ID. The
view borrows storage for the immediate read; it must be resolved again after
activation and cannot survive destruction/replacement of its owning bank.
The existing archive/bank integration now verifies reading both parameter
classes after staging and activation, signed zero despite a +0 cache, data
values, and switching back to the other bank. Write, listener transfer and
part-selection policies still belong to their actual backend owners.

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

## Full editor descriptor census

The live editor's registry contains 2,342 VST and 13,245 internal descriptors.
The read-only census completed without missing curves or failed rows. Of
these, 2,341 VST and 13,226 internal descriptor defaults use LinearPotardMapping.
MasterVolume uses BipolarVariableCenterInvPowPotardMapping, while 19 internal
descriptors use ExponentialPotardMapping. The class counts, examples and actual
master-volume curve fields are recorded in `resources/gui-mapping-census.json`.
These are descriptor defaults; widget custom mappings and display mappings
must still be resolved separately. The two unsupported classes are not replaced
with host-library approximations.

The live master-volume exponent is 2 and center is 0.5, but even a square/square
root substitution changes the reference results. A private comparison against
the reference machine's UCRT `powf` found 3,259 square and 81 square-root bit
differences in one million inputs within 0..1. The exact reference math path
must be recovered before enabling this mapping; field identity alone is not
sufficient.
