#include "mfrk/vcf37.hpp"
#include "vcf37_constants.hpp"
#include "vcf_sample.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>

namespace mfrk {

namespace {
using detail::vcf_constant;

struct DoubleSmoother {
    double target = 0.0;
    double current = 0.0;
    double coefficient;
    double threshold = std::bit_cast<double>(std::uint64_t{0x3d719799812dea11ull});
    explicit DoubleSmoother(float rate) {
        constexpr double time = std::bit_cast<double>(std::uint64_t{0x3f847ae140000000ull});
        coefficient = 1.0 - std::exp((-5.0 / (static_cast<double>(rate) * time)) * 32.0);
    }
    void set_both(float value) noexcept { target = current = static_cast<double>(value); }
    void set_target(float value) noexcept {
        if (target != static_cast<double>(value)) {
            target = static_cast<double>(value);
            threshold = std::max(std::abs(target) * 1e-12, 1e-20);
        }
    }
    float next() noexcept {
        const double delta = (target - current) * coefficient;
        if (std::abs(delta) >= threshold) current = current + delta;
        else current = target;
        return static_cast<float>(current);
    }
};

template<std::size_t Size>
float interpolate(const std::array<float, Size>& table, float position) noexcept {
    const auto index = static_cast<std::size_t>(position);
    return (table[index + 1] - table[index]) * (position - static_cast<float>(index)) + table[index];
}
}  // namespace

struct Vcf37::Impl : detail::VcfSampleState {
    float hz = 0.0f, g = 0.0f, r = 0.0f;
    DoubleSmoother g_smoother, r_smoother;

    Impl(float rate, std::uint32_t initial_seed)
        : VcfSampleState(rate, initial_seed, std::bit_cast<float>(std::uint32_t{0x2e4eb54cu})),
          g_smoother(rate), r_smoother(rate) {}

    float cutoff(float normalized) const noexcept {
        return interpolate(detail::kVcf37CutoffTable, normalized * vcf_constant<0x18055a2a4>());
    }
    float g_target(float frequency) const noexcept {
        const float n = frequency * inverse_rate;
        const float q = (n * n) * vcf_constant<0x18055a220>();
        return (vcf_constant<0x18055f7d4>() - (q * vcf_constant<0x180559ea4>() + vcf_constant<0x18055a028>()) * q) * n;
    }
    float r_target(float normalized) const noexcept {
        float adjusted = normalized;
        if (normalized > vcf_constant<0x180559fa8>()) {
            const float position = (normalized - vcf_constant<0x180559fa8>()) * vcf_constant<0x18055a300>();
            const float mapped = interpolate(detail::kVcf37ResonanceTable, position);
            const float h = hz * vcf_constant<0x1805573c0>();
            adjusted = (mapped - vcf_constant<0x180559fa8>()) * ((1.0f - h) * vcf_constant<0x180559f88>() + h) + vcf_constant<0x180559fa8>();
        }
        return vcf_constant<0x1805566a8>() - adjusted * vcf_constant<0x180556530>();
    }
};

Vcf37::Vcf37(float rate, std::uint32_t seed) : impl_(std::make_unique<Impl>(rate, seed)) {}
Vcf37::~Vcf37() = default;

void Vcf37::prepare(const Vcf37Controls& controls) noexcept {
    assert(controls.cutoff >= 0.0f && controls.cutoff <= 1.0f);
    assert(controls.resonance >= 0.0f && controls.resonance <= 1.0f);
    if (controls.first) {
        impl_->hz = impl_->cutoff(controls.cutoff);
        impl_->g_smoother.set_both(impl_->g_target(impl_->hz));
        impl_->r_smoother.set_both(impl_->r_target(controls.resonance));
    }
    if (controls.dirty[0] || controls.force) {
        impl_->hz = impl_->cutoff(controls.cutoff);
        impl_->g_smoother.set_target(impl_->g_target(impl_->hz));
    }
    if (controls.dirty[1] || controls.dirty[0] || controls.force) impl_->r_smoother.set_target(impl_->r_target(controls.resonance));
    if (controls.dirty[2] || controls.force) impl_->mode = controls.output;
    if (controls.dirty[3] || controls.force) impl_->noise = controls.noise;
}

void Vcf37::reset(float resonance) noexcept { impl_->reset(resonance); }

void Vcf37::process(std::span<const float, 32> input, std::span<float, 32> output) noexcept {
    impl_->g = impl_->g_smoother.next();
    impl_->r = impl_->r_smoother.next();
    for (std::size_t index = 0; index < input.size(); ++index) {
        output[index] = impl_->process_sample(input[index], impl_->g, impl_->r);
    }
}
Vcf37ControlSnapshot Vcf37::control_snapshot() const noexcept {
    return {impl_->hz, impl_->g, impl_->r, impl_->g_smoother.target, impl_->r_smoother.target, impl_->seed};
}

}  // namespace mfrk
