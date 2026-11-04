#include "mfrk/iir_resampler.hpp"
#include <algorithm>

namespace mfrk {
IirResampler::IirResampler(FilterOversampling factor) : factor_(factor) {}
void IirResampler::reset() noexcept {}
void IirResampler::upsample(std::span<const float>, std::span<float> output) noexcept { std::fill(output.begin(), output.end(), 0.0f); }
void IirResampler::downsample(std::span<const float>, std::span<float> output) noexcept { std::fill(output.begin(), output.end(), 0.0f); }
std::size_t IirResampler::factor() const noexcept { return static_cast<std::size_t>(factor_); }
std::uint32_t IirResampler::latency_frames() const noexcept { return 0; }
IirResamplerState IirResampler::state() const noexcept { return state_; }
}  // namespace mfrk
