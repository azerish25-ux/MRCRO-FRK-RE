#include "mfrk/virtual_analog_oscillator.hpp"

#include <array>

namespace mfrk {

VirtualAnalogOscillator::VirtualAnalogOscillator(float previous_sample)
    : upsampler_(previous_sample) {}

VirtualAnalogOscillator::VirtualAnalogOscillator(std::span<std::byte,128> scratch, float previous_sample)
    : engine_(scratch), upsampler_(previous_sample) {}

void VirtualAnalogOscillator::initialize() noexcept { engine_.initialize(); }

void VirtualAnalogOscillator::render(const VirtualAnalogParameters& parameters, float level,
                                     std::span<float, 32> output) noexcept {
    // The original wrapper always advances the engine, including muted blocks.
    std::array<float, 16> raw{};
    engine_.render(parameters, raw);
    upsampler_.process(raw, output);
    for (float& sample : output) {
        sample = level * sample;
    }
}

}  // namespace mfrk
