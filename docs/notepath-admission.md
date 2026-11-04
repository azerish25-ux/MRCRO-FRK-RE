# NotePath admission and actor contract

`mfrk::notepath::Engine::admitted_zero_arena()` explicitly supplies a zeroed preconstructor arena, followed by the recovered native constructor writes. The default constructor is inaccessible. This factory is a bounded reference profile, not a claim that native allocation or an upstream constructor zeroes memory.

Original row constructor 0x1802ba8d0 leaves source flags at byte 2 and gaps at bytes 9..11 and 19 unwritten. Whole NotePath constructor 0x18032ca70 preserves these bytes. The factory supplies zero for them, and later operations preserve or update them according to the source. Independent A5/zero probes and the ownership TODO are retained under ../local/evidence/notepath-review; source hashes are recorded in core/tests/notepath_provenance.json. The direct zero ranges in owner 0x1802f2160 stop before NotePath; earlier base construction and heap ownership remain unproved. Do not broaden the startup claim without evidence or caller-provided initial bytes.

Initial published-row storage has no native meaning: `published_valid_bits()` starts zero. Only observed callbacks establish valid snapshots. No initial MNF mirror contents are invented.

## Valid input boundary

The actor is a single-owner, fixed-size component. Calls and reads must not race. It does not parse raw host MIDI, normalize CC bytes, validate arbitrary memory, schedule strum/sequencer time or implement a voice/audio engine. Callers supply the recovered native inputs:

- `feed_midi_note`: primary pitch 0..127 and native note velocity. It follows the original primary note interface. Host note-on/zero-velocity translation and MIDI channel/MPE policy are outside this module.
- `feed`: a 58-byte canonical native packet with a valid primary pitch and an additional-note count 0..16. Native types 0/1/2 retain their original meaning; type 2 must not be converted into a generic MIDI note-on. Additional pitch 128 is the source sentinel; its original note-off alias is modeled. Other malformed pitch/count values are not admitted.
- Mode, allocation and unison getters: normalized Q15 values 0..32767. The external keyboard override retains the source nonzero test. Count preparation yields 2..6; the constructor's count 0 remains until explicitly prepared. No invented default is added.
- Hold: Q15 0..32767, or finite normalized float 0..1. Float conversion is the original float32 multiplication by 32767 followed by truncation. Thus normalized 0.5 is OFF (16383); raw Q15 16384 is ON. Dirty/force semantics are retained; no division by 128 or host CC normalization is inferred.
- Chord configuration uses native mode 0/1/2, a byte root (>=128 is the learning sentinel), learned count 0..8, at most eight interval bytes and total count 0..9. Interval arithmetic wraps as native bytes. Read learning results through `chord_state()`.
- Scale configuration uses native mode 0/1/2, type 0..7 and the native byte root. Supply the 12 mask/map entries from source-valid state; the original custom-map builder remains in the learning path. Read results through `scale_state()`. No generic nearest-scale substitution or arbitrary map validation is added.
- `PerNoteKey` is an opaque byte. It is not a MIDI/MPE/channel identifier. `NotePacket::per_note_key()` and `set_per_note_key()` preserve byte 0x38. The source primary interface itself writes the observed pitch there; this does not establish a universal semantic interpretation.

No new clamps or malformed-input fallback behavior were added. The original signed/unsigned comparisons, byte/word wrapping, source masks, duplicate suppression, extra-note indexing quirks and retained fields remain unchanged. Invalid configuration bytes can violate native indexing assumptions and must not be passed by an external adapter.

## Operation ordering

Every public operation starts a new `publications()` batch. Apply each record exactly once to the owning engine before any next public operation, including a configuration setter, dirty-skip hold call or `finish_control_block()`. Calling `publications()` does not acknowledge or drain it; repeated reads must not cause repeated application.

`published_rows()` plus `published_valid_bits()` is the latest observed snapshot, not a queue and not a command to replay status bytes each tick. It remains useful for inspection after the transient batch is replaced.

For a control block: dispatch each note/control operation and immediately apply its batch; run the concrete owner voice/control processing; consume the wrapping `note_counts()` values; then call `finish_control_block()`. That operation sets the source pending-status flag and clears both counts. The next primary note clears allocator row status bytes; the canonical interface also honors the native suppression flag.

`all_notes_off()` matches the recovered requested status-5 operation and retains shared hold. It is not a generic instrument panic/reset. Smoothing, envelopes, oscillator/filter state and host queues belong to other components.

The fixed 32-record publication buffer covers the admitted source operations (at most 17 records for a mono release packet; other public operations publish at most 12). There is no production allocation, I/O or dynamic queue. Build with C++20 and strict FP: `/fp:strict`, or `-fno-fast-math -ffp-contract=off`.
