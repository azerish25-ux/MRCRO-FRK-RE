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
}
