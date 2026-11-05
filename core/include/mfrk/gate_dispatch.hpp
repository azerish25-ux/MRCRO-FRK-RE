#pragma once
#include <array>
#include <cstdint>
#include "mfrk/notepath.hpp"
namespace mfrk::gate_dispatch {
enum class Kind:std::uint8_t { oscillator, main, cyclic };
struct Action {Kind kind;std::uint8_t voice,side;bool on;bool operator==(const Action&) const = default;};
struct Plan {std::array<Action,4> actions{};unsigned count=0;};
struct RowsPlan {std::array<Action,48> actions{};unsigned count=0;};
struct State {std::uint8_t active_count=0,first_row=0,last_row=0;std::array<std::uint8_t,6> first_sides{},last_sides{};};
// Gate projection of original 2dcb90 for one row with status 1..4. The
// owner must execute every action in order. A cyclic action also requires the
// original cyclic phase/reset work; this plan does not implement it. Main-off
// represents the source's inline envelope release/hold update. Pitch, LFO,
// expression, row ownership and other dispatcher effects are separate.
// Cyclic modes 0/1/2 and valid row/side state only; disabled suppresses cyclic.
bool dispatch(State&,std::uint8_t row,std::uint8_t status,bool para,
              std::uint8_t cyclic_mode,bool cyclic_disabled,Plan&) noexcept;
// Scan in native voice-then-side order. Only row statuses 0..4 are admitted;
// caller retains the rows and clears their status after the full voice stage.
bool plan_rows(State&,const std::array<notepath::NoteRow,12>&,bool para,
               std::uint8_t cyclic_mode,bool cyclic_disabled,RowsPlan&) noexcept;
}
