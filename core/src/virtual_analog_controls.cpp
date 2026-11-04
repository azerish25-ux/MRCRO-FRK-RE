#include "mfrk/virtual_analog_controls.hpp"

#include <bit>

namespace mfrk {

void update_virtual_analog_controls(VirtualAnalogControlState& state) noexcept {
    // Original coefficient at 0x180555eac; preserve its exact float32 value.
    constexpr float scale = std::bit_cast<float>(std::uint32_t{0x37800200u});
    if (state.dirty[0] != 0) {
        state.parameters.harmonics = static_cast<float>(state.words[0]) * scale;
    }
    if (state.dirty[1] != 0) {
        state.parameters.timbre = static_cast<float>(state.words[1]) * scale;
    }
    if (state.dirty[2] != 0) {
        state.parameters.morph = static_cast<float>(state.words[2]) * scale;
    }
    if (state.latched == 0) {
        if (state.signal != 0) {
            state.signal = 0;
            state.latched = 1;
            state.trigger = 1;
            return;
        }
    } else if (state.signal == 0) {
        state.latched = 0;
    }
    state.trigger = 0;
}

}  // namespace mfrk
