#include "mfrk/oversampled_vcf49.hpp"
namespace mfrk {
OversampledVcf49::OversampledVcf49(float rate, FilterOversampling factor,
                                 std::size_t frames, std::uint32_t seed, FilterKernel kernel)
    : resource_(factor, kernel, frames), filter_(rate * static_cast<float>(factor), seed) {}

void OversampledVcf49::prepare(const VcfControls& controls) noexcept { filter_.prepare(controls); }

void OversampledVcf49::reset(float resonance) noexcept {
    resource_.reset();
    filter_.reset(resonance);
}

void OversampledVcf49::process(std::span<const float> audio, std::span<const float> modulation,
                              std::span<float> output) noexcept {
    resource_.process(filter_, audio, modulation, output);
}

std::uint32_t OversampledVcf49::noise_state() const noexcept { return filter_.noise_state(); }
}  // namespace mfrk
