#pragma once

#include <cstdint>
#include <array>

namespace mfrk {
enum class VcfOutput : std::uint8_t { lowpass, bandpass, highpass };

struct VcfControls {
    float cutoff;
    float resonance;
    VcfOutput output;
    bool noise;
    std::array<std::uint8_t, 4> dirty;
    bool first;
    bool force;
};
}  // namespace mfrk
