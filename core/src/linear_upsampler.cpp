#include "mfrk/linear_upsampler.hpp"

#include <cstddef>

namespace mfrk {

void LinearUpsampler2x::process(std::span<const float, 16> input,
                                std::span<float, 32> output) noexcept {
    for (std::size_t index = 0; index < input.size(); ++index) {
        const float current = input[index];
        output[index * 2] = (current + previous_) * 0.5f;
        output[index * 2 + 1] = current;
        previous_ = current;
    }
}

}  // namespace mfrk
