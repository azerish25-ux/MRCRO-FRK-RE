#include "mfrk/virtual_analog_oscillator.hpp"

#include <algorithm>

namespace mfrk {

VirtualAnalogOscillator::VirtualAnalogOscillator(float previous_sample)
    : upsampler_(previous_sample) {}

// RED phase: original wrapper references must reject this.
void VirtualAnalogOscillator::render(const VirtualAnalogParameters&, float,
                                     std::span<float, 32> output) noexcept {
    std::fill(output.begin(), output.end(), 0.0f);
}

}  // namespace mfrk
