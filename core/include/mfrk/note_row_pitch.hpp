#pragma once
#include "mfrk/notepath.hpp"
#include "mfrk/pitch_update.hpp"
#include <array>
#include <cstdint>
namespace mfrk::note_row_pitch {
struct Records {std::array<std::uint16_t,12> words{};};
// Pitch-related projection of 2dcb90 for row statuses 0..5, 7-bit notes and
// fixed 0..128 pitch descriptors. Does not perform gate/envelope/LFO effects.
// Active count is the value at entry; this computes its resulting value too.
bool collect(Records&,pitch_update::GlideControl&,std::uint8_t& active_count,
             const std::array<notepath::NoteRow,12>&,bool para) noexcept;
// Source 2d4f70 applies dirty note records in voice-then-side order. Dirty
// flags remain set until the caller's later control-block clear boundary.
bool apply_pending(const Records&,pitch_update::State&,pitch_update::GlideControl&) noexcept;
void clear_note_dirty(pitch_update::GlideControl&) noexcept;
}
