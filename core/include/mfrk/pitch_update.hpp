#pragma once
#include <array>
#include <cstdint>
namespace mfrk::pitch_update {
struct Slot {float note=0;std::uint8_t dirty=0;};
// Source field profile for six voices and two sides. Array index is voice+6*side;
// slots are ordered voice*2+side. All supplied floats/doubles must be finite.
// This is the 2da0a0 update, not the upstream note/glide setup or full voice.
struct State {
 std::uint8_t split=0;
 std::array<std::uint8_t,12> finished{},dirty{};
 std::array<float,12> target{};
 std::array<double,12> position{},increment{};
 float tune=0,bend=0;
 std::array<float,2> side_a{},side_b{};
 std::array<float,12> output{};
 std::array<Slot,12> slots{};
};
void advance(State&) noexcept;
// Raw source fields used by 2da290; names describe observed storage roles,
// not public parameter mappings. Rates must be finite and positive when used.
struct GlideControl {
 std::uint8_t scale_mode=0,gate_mode=0,shared_gate=0,alternate=0,force=0;
 std::uint32_t last_index=0;
 std::array<std::uint8_t,12> enabled{},held{},dirty_a{},dirty_b{};
 std::array<float,12> rate_a{},rate_b{};
};
// For shared mode index must be 0..5; for split mode 0..11. On rejected
// index/rate the state is unchanged. Other supplied numeric fields are finite.
bool setup_glide(State&,GlideControl&,unsigned index,bool copy_previous) noexcept;
// Active note profile: original 0..128 descriptor encodes MIDI pitch to a
// 16-bit record, then the pitch controller decodes that record and initializes
// glide. Voice selection, triggering and other note-row side effects remain with
// the owner. Invalid pitch/index or unsupported glide inputs leave state intact.
bool encode_note_pitch(std::uint8_t pitch,std::uint16_t& word) noexcept;
bool apply_note_pitch(State&,GlideControl&,unsigned index,std::uint8_t pitch) noexcept;
}
