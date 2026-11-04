#pragma once

#include "mfrk/virtual_analog.hpp"

#include <array>
#include <cstdint>

namespace mfrk {

// Caller-owned recovered fields, not the full native object's binary layout.
// Word/dirty order: harmonics, timbre, morph. Byte flags preserve nonzero values
// exactly where the original routine leaves them unchanged.
struct VirtualAnalogControlState {
    std::array<std::uint16_t, 3> words{};
    std::array<std::uint8_t, 3> dirty{};
    std::uint8_t signal = 0;
    std::uint8_t latched = 0;
    std::int32_t trigger = 0;
    VirtualAnalogParameters parameters{};
};

void update_virtual_analog_controls(VirtualAnalogControlState& state) noexcept;

}  // namespace mfrk
