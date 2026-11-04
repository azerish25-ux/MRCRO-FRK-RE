#include "mfrk/oversampled_vcf49.hpp"
#include <algorithm>

namespace mfrk {
OversampledVcf49::OversampledVcf49(float rate, FilterOversampling factor,
                                 std::size_t frames, std::uint32_t seed)
    : audio_resampler_(factor), modulation_resampler_(factor),
      filter_(rate * static_cast<float>(factor), seed), maximum_frames_(frames) {}
void OversampledVcf49::prepare(const VcfControls&) noexcept {}
void OversampledVcf49::reset(float) noexcept {}
void OversampledVcf49::process(std::span<const float>, std::span<const float>, std::span<float> output) noexcept { std::fill(output.begin(), output.end(), 0.0f); }
std::uint32_t OversampledVcf49::noise_state() const noexcept { return 0; }
}  // namespace mfrk
