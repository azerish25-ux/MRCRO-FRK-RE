#include "mfrk/vcf37.hpp"
#include "vcf37_constants.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>

namespace mfrk {

namespace {
using detail::vcf_constant;

float feedback_curve(float x) noexcept {
    // The owning core forms abs(drive)*positiveScale+a0. Earlier branches of the
    // native general helper are unreachable in this model37 call domain.
    assert(x >= std::bit_cast<float>(std::uint32_t{0xc08d3b9du}));
    float w;
    if (x <= vcf_constant<0x18055f734>()) {
        if (x <= vcf_constant<0x18055f7d0>()) {
            w = (x * vcf_constant<0x180559b24>() + vcf_constant<0x180559d18>()) /
                ((x * vcf_constant<0x180559e1c>() + vcf_constant<0x180559f4c>()) * x + 1.0f);
        } else {
            w = ((x * vcf_constant<0x180559a90>() + vcf_constant<0x180559d94>()) * x + vcf_constant<0x180559e84>()) * x + vcf_constant<0x180559f24>();
        }
    } else if (x <= vcf_constant<0x18055a2e4>()) {
        if (x <= vcf_constant<0x18055a1c0>()) {
            w = ((vcf_constant<0x180559d70>() - x * vcf_constant<0x1805597f0>()) * x + vcf_constant<0x180559e80>()) * x + vcf_constant<0x180559f20>();
        } else {
            w = ((vcf_constant<0x180559b50>() - x * vcf_constant<0x180557a20>()) * x + vcf_constant<0x180559f74>()) * x - vcf_constant<0x180559ddc>();
        }
    } else if (x <= vcf_constant<0x18055a3ac>()) {
        w = ((vcf_constant<0x180558064>() - x * vcf_constant<0x18055492c>()) * x + vcf_constant<0x180559fc4>()) * x - vcf_constant<0x18055a120>();
    } else {
        w = x - vcf_constant<0x18055a42c>() / x;
    }
    const auto word = std::bit_cast<std::uint32_t>(w);
    const float q = std::bit_cast<float>((word & 0x7fffffu) | 0x3f800000u) - 1.0f;
    const float logarithm = (((q * vcf_constant<0x180559dc0>() - vcf_constant<0x180559ea8>()) * q + 1.0f) * q) +
        static_cast<float>(static_cast<int>(word >> 23) - 127) * vcf_constant<0x180559f50>();
    const float error = (x - w) - logarithm;
    const float wp = w + 1.0f;
    const float d = (error * vcf_constant<0x180559f40>() + wp) * wp;
    return (((d - error * vcf_constant<0x180559ee8>()) * error) / ((d - error) * wp) + 1.0f) * w;
}

float saturate(float x) noexcept {
    if (x < 0.0f) {
        if (x >= vcf_constant<0x18055f764>()) return x - (vcf_constant<0x18055f728>() - x * vcf_constant<0x18055f71c>()) * (x * x);
        if (x >= vcf_constant<0x18055f7a8>()) return (vcf_constant<0x18055a064>() - (vcf_constant<0x18055f740>() - x * vcf_constant<0x180559d90>()) * x) * x - vcf_constant<0x18055f70c>();
        return (x - vcf_constant<0x18055f780>()) / (vcf_constant<0x18055f77c>() - x);
    }
    if (x <= 1.0f) return x - (x * vcf_constant<0x180559db0>() + vcf_constant<0x180559de4>()) * (x * x);
    if (x <= 2.0f) return ((x * vcf_constant<0x180559d90>() - vcf_constant<0x180559f14>()) * x + vcf_constant<0x18055a064>()) * x - vcf_constant<0x180559d08>();
    return (x - vcf_constant<0x18055a0a8>()) / (x - vcf_constant<0x18055a0a0>());
}

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

struct Vcf37::Impl {
    float inverse_rate;
    float compensation;
    float dc_coefficient;
    float feedforward;
    float negative_feedforward;
    float dc_state = 0.0f;
    float baseline = saturate(std::bit_cast<float>(std::uint32_t{0x3c83126fu}));
    float offset = std::bit_cast<float>(std::uint32_t{0xc08d3b9du});
    float s1 = 0.0f, s2 = 0.0f, delayed_bp = 0.0f, feedback = 0.0f;
    float hz = 0.0f, g = 0.0f, r = 0.0f;
    VcfOutput mode = VcfOutput::lowpass;
    bool noise = true;
    std::uint32_t seed;
    DoubleSmoother g_smoother, r_smoother;

    Impl(float rate, std::uint32_t initial_seed)
        : inverse_rate(1.0f / rate), seed(initial_seed), g_smoother(rate), r_smoother(rate) {
        const float capacitor = std::bit_cast<float>(std::uint32_t{0x2e4eb54cu});
        const float c2 = capacitor * capacitor;
        float q = rate * rate;
        q = q * vcf_constant<0x18055a558>();
        q = q * c2;
        q = 1.0f / q;
        q = q + 1.0f;
        q = std::sqrt(q);
        compensation = 1.0f / q;
        dc_coefficient = std::exp(inverse_rate * std::bit_cast<float>(std::uint32_t{0xc1490fdbu}));
        feedforward = (dc_coefficient + 1.0f) * 0.5f;
        negative_feedforward = -feedforward;
    }

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

void Vcf37::reset(float resonance) noexcept {
    if (resonance < 0.98f) impl_->s1 = impl_->s2 = impl_->delayed_bp = 0.0f;
    else {
        const float value = (resonance - 0.98f) * 10.5f;
        if (impl_->mode == VcfOutput::bandpass) {
            impl_->s1 = impl_->delayed_bp = 0.0f;
            impl_->s2 = value;
        } else {
            impl_->s1 = impl_->delayed_bp = value;
            impl_->s2 = 0.0f;
        }
    }
    impl_->feedback = impl_->dc_state = 0.0f;
}

void Vcf37::process(std::span<const float, 32> input, std::span<float, 32> output) noexcept {
    impl_->g = impl_->g_smoother.next();
    impl_->r = impl_->r_smoother.next();
    const float g = impl_->g, r = impl_->r;
    constexpr float a = vcf_constant<0x180559ec4>();
    constexpr float b = vcf_constant<0x180559da0>();
    constexpr float k = vcf_constant<0x18055a4c8>();
    for (std::size_t index = 0; index < input.size(); ++index) {
        float sample = input[index];
        if (impl_->noise) {
            auto seed = impl_->seed ^ (impl_->seed << 13);
            seed ^= seed >> 17;
            seed ^= seed << 5;
            impl_->seed = seed;
            sample = (static_cast<float>(seed) * vcf_constant<0x180554834>() + sample) - vcf_constant<0x1805587e8>();
        }
        const float numerator = ((((r * k - (g * a + b)) * impl_->s1 + sample * vcf_constant<0x180559e8c>()) - impl_->s2 * a) + (impl_->s1 - impl_->delayed_bp) * impl_->compensation) + impl_->feedback * k;
        const float denominator = (((g * g) * a + g * b) + a) - ((g * k) * r) - impl_->compensation * g;
        const float hp = numerator / denominator;
        const float g_hp = hp * g;
        const float bp = g_hp + impl_->s1;
        const float g_bp = bp * g;
        const float lp = g_bp + impl_->s2;
        impl_->s1 = bp + g_hp;
        impl_->s2 = lp + g_bp;
        impl_->delayed_bp = bp;
        const float drive = bp * vcf_constant<0x180559f68>() - ((hp + lp) * a + bp * b);
        const float x = std::abs(drive) * vcf_constant<0x18055a2c4>() + impl_->offset;
        const float sign = std::bit_cast<float>((std::bit_cast<std::uint32_t>(drive) & 0xbf800000u) | 0x3f800000u);
        impl_->feedback = (feedback_curve(x) * vcf_constant<0x180554a1c>() - vcf_constant<0x180554890>()) * sign;
        const float selected = impl_->mode == VcfOutput::lowpass ? lp : impl_->mode == VcfOutput::bandpass ? bp : impl_->mode == VcfOutput::highpass ? hp : 0.0f;
        const float raw = (saturate(vcf_constant<0x180559c34>() - selected * vcf_constant<0x180559e28>()) - impl_->baseline) * vcf_constant<0x18055a1e8>();
        const float y = raw * impl_->feedforward + impl_->dc_state;
        impl_->dc_state = y * impl_->dc_coefficient + raw * impl_->negative_feedforward;
        output[index] = y * vcf_constant<0x18055a048>();
    }
}
Vcf37ControlSnapshot Vcf37::control_snapshot() const noexcept {
    return {impl_->hz, impl_->g, impl_->r, impl_->g_smoother.target, impl_->r_smoother.target, impl_->seed};
}

}  // namespace mfrk
