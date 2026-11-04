#pragma once
#include <array>
#include <cstdint>

namespace mfrk::notepath {
inline constexpr unsigned voice_rows = 12;
inline constexpr unsigned packet_bytes = 58;
// Opaque native per-note table key. No MIDI/MPE/channel meaning is inferred.
struct PerNoteKey { std::uint8_t value; };
struct NotePacket {
  std::array<std::uint8_t, packet_bytes> bytes{};
  [[nodiscard]] PerNoteKey per_note_key() const noexcept { return {bytes[0x38]}; }
  void set_per_note_key(PerNoteKey key) noexcept { bytes[0x38] = key.value; }
};
struct NoteRow { std::array<std::uint8_t, 20> bytes{}; };
struct Publication { std::uint8_t row; NoteRow value; };
struct Publications {
  std::array<Publication, 32> entries{};
  unsigned count = 0;
};
struct ChordState {
  std::uint8_t mode = 0, root = 60, learned = 0, count = 0;
  std::array<std::uint8_t, 8> intervals{};
};
struct ScaleState {
  std::uint8_t mode = 0, type = 0, root = 0;
  std::array<std::uint8_t, 12> mask{}, map{};
};
struct ModeControls {
  std::int16_t note_mode_q15, allocation_q15, unison_mode_q15;
  // Native keyboard getter0x12 (Kbd_Chord_En), distinct from the sequencer facet.
  std::int16_t external_mode_override;
};
struct HoldDispatch {
  bool processed;
  std::uint16_t control_id;
  std::int16_t value;
};
struct TestAccess;
struct NoteCounts { std::uint8_t on, off; };
// Fixed-size single-owner actor; not a thread-safe host MIDI parser.
// Supply source-valid native states/packets and normalized controls. See docs/notepath-admission.md.
class Engine {
  struct PolyState {
    std::uint8_t enabled = 0, priority = 2, legato = 1;
    std::array<std::uint16_t, 128> age{};
    std::array<std::uint8_t, 128> velocity{};
    std::uint16_t counter = 1;
    std::uint8_t selected = 128, source = 0;
    NotePacket pending;
  };

  struct UnisonState {
    std::uint8_t enabled = 0, mode = 0, count = 0, held = 0;
  };

  struct AllocatorState {
    std::array<NoteRow, voice_rows> rows{};
    std::uint8_t capacity = 6, policy = 0, allocation = 0, steal = 0;
    std::uint8_t selected = 0, legato = 0, parity = 0, unison_table = 0;
    std::uint8_t duplicates = 0, cursor = 255;
    std::uint32_t age_counter = 0;
    std::uint16_t active_bits = 0;
    std::uint8_t sequence_index = 0;
    std::array<std::uint16_t, 128> pitch_table{};
  };

  struct State {
    PolyState poly;
    ChordState chord;
    UnisonState unison;
    ScaleState scale;
    AllocatorState allocator;
    NotePacket last_input;
    std::uint8_t held = 0, last_active_row = 0;
    std::uint16_t keyboard_scale_mask = 0, global_scale_mask = 0;
    std::uint8_t global_scale = 0, global_root = 0;
    std::uint8_t status_pending = 0, suppress_status_clear = 0;
    std::uint8_t on_count = 0, off_count = 0;
  };

  struct AllocatorEvent {
    std::uint8_t type, pitch, velocity, flags, auxiliary;
    std::uint16_t word;
    // Original per-note table key. It is not assumed to be an MPE/MIDI channel.
    std::uint8_t key;
  };
  struct AdmittedZeroArenaTag {};
  explicit Engine(AdmittedZeroArenaTag) noexcept;
public:
  // Explicit ZERO preconstructor arena. Original row ctor does not write flags
  // byte2 or gaps9..11/19. This is an admitted profile, not a native heap claim.
  [[nodiscard]] static Engine admitted_zero_arena() noexcept {
    return Engine(AdmittedZeroArenaTag{});
  }
  [[nodiscard]] NoteCounts note_counts() const noexcept {
    return {state_.on_count, state_.off_count};
  }
  // Read back native learning/configuration state without exposing actor internals.
  [[nodiscard]] const ChordState& chord_state() const noexcept { return state_.chord; }
  [[nodiscard]] const ScaleState& scale_state() const noexcept { return state_.scale; }
  [[nodiscard]] bool hold_enabled() const noexcept { return state_.held != 0; }
  [[nodiscard]] std::uint8_t last_active_row() const noexcept { return state_.last_active_row; }
  // Apply this batch ONCE before ANY next public operation, including finish.
  // A new operation overwrites it; entries are snapshots, not a replay queue.
  [[nodiscard]] const Publications& publications() const noexcept { return events_; }
  [[nodiscard]] const std::array<NoteRow, voice_rows>& published_rows() const noexcept {
    return published_;
  }
  // A published row has meaning only after its validity bit is set by a callback.
  [[nodiscard]] std::uint16_t published_valid_bits() const noexcept { return published_valid_; }
  void prepare_mode(ModeControls) noexcept;
  void prepare_unison_count(std::int16_t q15) noexcept;
  void prepare_legato(std::int16_t q15) noexcept;
  // Source-state configuration: chordcount0..9/learned0..8, scale type0..7;
  // see ADMISSION.md for the complete valid-input contract.
  void configure_chord(ChordState) noexcept;
  void configure_scale(ScaleState) noexcept;
  void feed_midi_note(bool on, std::uint8_t pitch, std::uint8_t velocity) noexcept;
  // Canonical packet route through the original secondary NotePath interface.
  void feed(NotePacket&) noexcept;
  [[nodiscard]] HoldDispatch update_hold(std::int16_t q15, bool dirty = true,
                                        bool force = false) noexcept;
  // Preconditions: finite normalized0..1; q15 controls0..32767. No host CC
  // byte normalization is inferred. No arbitrary-byte state validation/clamping.
  [[nodiscard]] HoldDispatch update_hold_normalized(float normalized, bool dirty = true,
                                                   bool force = false) noexcept;
  // Matches requested all-notes-off32da70/status5, retaining the shared held flag.
  void all_notes_off(bool requested = true) noexcept;
  // Original owner2f3b0d..2f3b1f: after voice processing/note-count consumption.
  void finish_control_block() noexcept;
private:
  friend struct TestAccess;
  using RowCallback = void (*)(void*, std::uint8_t, const NoteRow&) noexcept;
  static void advance_age(AllocatorState&, unsigned row) noexcept;
  static std::uint8_t choose_note(const PolyState&) noexcept;
  static NotePacket empty_packet() noexcept;
  static void reset_poly(PolyState&) noexcept;
  static bool poly_process(PolyState&, NotePacket&) noexcept;
  static bool chord_process(ChordState&, NotePacket&) noexcept;
  static bool unison_process(UnisonState&, NotePacket&) noexcept;
  static bool scale_process(ScaleState&, NotePacket&) noexcept;
  static void build_custom_map(ScaleState&) noexcept;
  static std::uint8_t free_slot(AllocatorState&, std::uint8_t, std::uint8_t) noexcept;
  static std::uint8_t steal_slot(const AllocatorState&, std::uint8_t) noexcept;
  static bool allocator_event(AllocatorState&, AllocatorEvent, RowCallback, void*) noexcept;
  void pipeline(NotePacket&) noexcept;
  void allocator_batch(NotePacket&) noexcept;
  void publish(std::uint8_t, const NoteRow&) noexcept;
  static void on_row(void*, std::uint8_t, const NoteRow&) noexcept;
  void clear_row(std::uint8_t, std::uint8_t status) noexcept;
  void clear_statuses() noexcept;
  void begin_operation() noexcept { events_.count = 0; }
  State state_;
  Publications events_;
  std::array<NoteRow, voice_rows> published_{};
  std::uint16_t published_valid_ = 0;
};
}
