#pragma once

#include "mfrk/linear_upsampler.hpp"
#include "mfrk/virtual_analog.hpp"

namespace mfrk {

// Recovered raw-engine, interpolation and supplied-level stages. Factory
// defaults, control conversion, voices, filters and effects are separate.
class VirtualAnalogOscillator {
public:
    explicit VirtualAnalogOscillator(float previous_sample = 0.0f);
    explicit VirtualAnalogOscillator(std::span<std::byte,128> scratch, float previous_sample = 0.0f);

    // Selection Init resets engine state only, never upsampler history.
    void initialize() noexcept;

    void render(const VirtualAnalogParameters& parameters, float level,
                std::span<float, 32> output) noexcept;

private:
    VirtualAnalog engine_;
    LinearUpsampler2x upsampler_;
};

}  // namespace mfrk
