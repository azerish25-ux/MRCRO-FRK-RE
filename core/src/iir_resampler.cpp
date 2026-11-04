#include "mfrk/iir_resampler.hpp"
#include "iir_coefficients.hpp"

#include <cassert>
#include <stdexcept>

namespace mfrk {
namespace {
const detail::IirCoefficients& descriptor(FilterOversampling factor) {
    switch (factor) {
    case FilterOversampling::x2: return detail::kFilterIirCoefficients[0];
    case FilterOversampling::x4: return detail::kFilterIirCoefficients[1];
    case FilterOversampling::x6: return detail::kFilterIirCoefficients[2];
    case FilterOversampling::x8: return detail::kFilterIirCoefficients[3];
    }
    throw std::invalid_argument("Unsupported original filter oversampling factor");
}

float section_chain(float input, std::array<float, 16>& state,
                    const detail::IirCoefficients& coefficients) noexcept {
    for (std::size_t i = 0; i < coefficients.numerator.size(); ++i) {
        const float output = input + state[i * 2];
        state[i * 2] = (input * coefficients.numerator[i] - output * coefficients.denominator[i * 2]) + state[i * 2 + 1];
        state[i * 2 + 1] = input - output * coefficients.denominator[i * 2 + 1];
        input = output;
    }
    return input;
}
}  // namespace

IirResampler::IirResampler(FilterOversampling factor) : factor_(factor) { (void)descriptor(factor_); }
void IirResampler::reset() noexcept { state_ = {}; }

void IirResampler::upsample(std::span<const float> input, std::span<float> output) noexcept {
    const auto ratio = factor();
    assert(output.size() % ratio == 0 && output.size() / ratio == input.size());
    const auto& coefficients = descriptor(factor_);
    const float gain = static_cast<float>(ratio) * coefficients.gain;
    std::size_t index = 0;
    for (float sample : input) {
        for (std::size_t phase = 0; phase < ratio; ++phase) {
            const float value = phase == 0 ? sample * gain : 0.0f;
            output[index++] = section_chain(value, state_.up, coefficients);
        }
    }
}

void IirResampler::downsample(std::span<const float> input, std::span<float> output) noexcept {
    const auto ratio = factor();
    assert(input.size() % ratio == 0 && input.size() / ratio == output.size());
    const auto& coefficients = descriptor(factor_);
    for (std::size_t i = 0; i < output.size(); ++i) {
        for (std::size_t phase = 0; phase < ratio; ++phase) {
            const float value = section_chain(input[i * ratio + phase] * coefficients.gain, state_.down, coefficients);
            // The original selects phase0 but still advances every later phase.
            if (phase == 0) output[i] = value;
        }
    }
}

std::size_t IirResampler::factor() const noexcept { return static_cast<std::size_t>(factor_); }
std::uint32_t IirResampler::latency_frames() const noexcept { return descriptor(factor_).latency; }
IirResamplerState IirResampler::state() const noexcept { return state_; }
}  // namespace mfrk
