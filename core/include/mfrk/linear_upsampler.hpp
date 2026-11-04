#pragma once

#include <span>

namespace mfrk {

// Recovered 16-to-32-frame operation. Input/output must not overlap. Initial
// history is supplied by the caller; this does not assert the instrument's
// complete initialization sequence.
class LinearUpsampler2x {
public:
    explicit constexpr LinearUpsampler2x(float previous = 0.0f) noexcept
        : previous_(previous) {}

    void process(std::span<const float, 16> input,
                 std::span<float, 32> output) noexcept;

private:
    float previous_;
};

}  // namespace mfrk
