#pragma once

#include "mfrk/vcf_types.hpp"

namespace mfrk::detail {

// Shared nonlinear state, separate from model-specific control/modulation maps.
// Capacitor is the exact model-specific constant recovered from initialization.
struct VcfSampleState {
    VcfSampleState(float rate, std::uint32_t initial_seed, float capacitor);
    void reset(float resonance) noexcept;
    float process_sample(float input, float g, float r) noexcept;

    float inverse_rate;
    float compensation;
    float dc_coefficient;
    float feedforward;
    float negative_feedforward;
    float dc_state = 0.0f;
    float baseline;
    float offset;
    float s1 = 0.0f, s2 = 0.0f, delayed_bp = 0.0f, feedback = 0.0f;
    VcfOutput mode = VcfOutput::lowpass;
    bool noise = true;
    std::uint32_t seed;
};

}  // namespace mfrk::detail
