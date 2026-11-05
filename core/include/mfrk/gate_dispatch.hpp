#pragma once
#include <array>
#include <cstdint>
#include "mfrk/notepath.hpp"
namespace mfrk::gate_dispatch {
enum class Kind:std::uint8_t { oscillator, main, cyclic, main_reset, cyclic_reset, lfo1_reset, lfo2_reset, voice_reset };
struct Action {Kind kind;std::uint8_t voice,side;bool on;bool operator==(const Action&) const = default;};
struct Plan {std::array<Action,6> actions{};unsigned count=0;};
struct RowsPlan {std::array<Action,72> actions{};unsigned count=0;};
struct State {std::uint8_t active_count=0,first_row=0,last_row=0;std::array<std::uint8_t,6> first_sides{},last_sides{};};
struct ResetContext {bool other_active,other_enabled;};
// Gate projection of original 2dcb90 for status 1..4, plus status 5 in shared
// mode; para status 5 requires the explicit other-lane reset context.
// The owner must execute every action in order. A cyclic action also requires the
// original cyclic phase/reset work; this plan does not implement it. Main-off
// represents the source's inline envelope release/hold update. Pitch, LFO,
// expression, row ownership and other dispatcher effects are separate.
// Cyclic modes 0/1/2 and valid row/side state only; disabled suppresses cyclic.
bool dispatch(State&,std::uint8_t row,std::uint8_t status,bool para,
              std::uint8_t cyclic_mode,bool cyclic_disabled,Plan&,const ResetContext* reset_context=nullptr) noexcept;
// Scan in native voice-then-side order within the same status profile;
// caller retains the rows and clears their status after the full voice stage.
bool plan_rows(State&,const std::array<notepath::NoteRow,12>&,bool para,
               std::uint8_t cyclic_mode,bool cyclic_disabled,RowsPlan&,const std::uint16_t* row_mask=nullptr) noexcept;
}
