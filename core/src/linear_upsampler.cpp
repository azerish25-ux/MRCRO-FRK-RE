#include "mfrk/linear_upsampler.hpp"

#include <algorithm>

namespace mfrk {

// RED phase: direct original-instruction and composed-engine checks must reject
// this before the recovered operation is implemented.
void LinearUpsampler2x::process(std::span<const float, 16>,
                                std::span<float, 32> output) noexcept {
    std::fill(output.begin(), output.end(), 0.0f);
}

}  // namespace mfrk
