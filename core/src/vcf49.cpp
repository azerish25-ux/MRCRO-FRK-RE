#include "mfrk/vcf49.hpp"
#include "vcf37_constants.hpp"
#include "vcf49_tables.hpp"
#include "vcf_sample.hpp"

#include <algorithm>
#include <cassert>

namespace mfrk {
namespace {
using detail::vcf_constant;

template<std::size_t Size>
float interpolate(const std::array<float, Size>& table, float position) noexcept {
    const auto index = static_cast<std::size_t>(position);
    return (table[index + 1] - table[index]) * (position - static_cast<float>(index)) + table[index];
}
}  // namespace

struct Vcf49::Impl : detail::VcfSampleState {
    float cutoff = 0.0f, resonance = 0.0f;
    Impl(float rate, std::uint32_t seed)
        : VcfSampleState(rate, seed, vcf_constant<0x18055484c>()) {}

    float coefficient_g(float hz) const noexcept {
        const float n = hz * inverse_rate;
        const float q = (n * n) * vcf_constant<0x18055a220>();
        return (vcf_constant<0x18055f7d4>() - (q * vcf_constant<0x180559ea4>() + vcf_constant<0x18055a028>()) * q) * n;
    }

    float coefficient_r(float hz) const noexcept {
        float adjusted = resonance;
        if (resonance > vcf_constant<0x180559fd0>() &&
            hz > vcf_constant<0x18055a490>() && hz < vcf_constant<0x18055a4c4>()) {
            const float scale = (hz - vcf_constant<0x18055a490>()) * vcf_constant<0x1805591fc>();
            const float value = interpolate(detail::kVcf49Resonance, scale);
            adjusted = (((value - 1.0f) * vcf_constant<0x18055a308>()) *
                        (resonance - vcf_constant<0x180559fd0>()) + 1.0f) * resonance;
        }
        return vcf_constant<0x1805566a8>() - adjusted * vcf_constant<0x180556530>();
    }
};

Vcf49::Vcf49(float rate, std::uint32_t seed) : impl_(std::make_unique<Impl>(rate, seed)) {}
Vcf49::~Vcf49() = default;

void Vcf49::prepare(const VcfControls& controls) noexcept {
    assert(controls.cutoff >= 0.0f && controls.cutoff <= 1.0f);
    assert(controls.resonance >= 0.0f && controls.resonance <= 1.0f);
    // Original model49 render reads normalized parameter records directly each
    // block, bypassing the scalar/double caches used by the model37 path.
    impl_->cutoff = controls.cutoff;
    impl_->resonance = controls.resonance;
    if (controls.dirty[2] || controls.force) impl_->mode = controls.output;
    if (controls.dirty[3] || controls.force) impl_->noise = controls.noise;
}

void Vcf49::reset(float resonance) noexcept { impl_->reset(resonance); }

void Vcf49::process(std::span<const float> audio, std::span<const float> modulation,
                    std::span<float> output) noexcept {
    assert(audio.size() == modulation.size() && audio.size() == output.size());
    const float base_vco = interpolate(detail::kVcf49BaseVco, impl_->cutoff * 128.0f);
    for (std::size_t i = 0; i < audio.size(); ++i) {
        const float normalized = std::clamp(modulation[i] * vcf_constant<0x18055a00c>() + impl_->cutoff, 0.0f, 1.0f);
        float vco = modulation[i] * vcf_constant<0x18055a518>() + base_vco;
        vco = std::clamp(vco, vcf_constant<0x18055a38c>(), vcf_constant<0x18055a500>());
        const float multiplier = interpolate(detail::kVcf49Multiplier, normalized * 40.0f);
        vco = std::min(vcf_constant<0x18055a500>(), multiplier * vco);
        const float hz = interpolate(detail::kVcf49Frequency, vco * (1.0f / 1024.0f));
        output[i] = impl_->process_sample(audio[i], impl_->coefficient_g(hz), impl_->coefficient_r(hz));
    }
}

std::uint32_t Vcf49::noise_state() const noexcept { return impl_->seed; }
}  // namespace mfrk
