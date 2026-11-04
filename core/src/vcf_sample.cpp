#include "vcf_sample.hpp"
#include "vcf37_constants.hpp"

#include <bit>
#include <cassert>
#include <cmath>

namespace mfrk::detail {
namespace {
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

}  // namespace

VcfSampleState::VcfSampleState(float rate, std::uint32_t initial_seed, float capacitor)
    : inverse_rate(1.0f / rate),
      baseline(saturate(std::bit_cast<float>(std::uint32_t{0x3c83126fu}))),
      offset(std::bit_cast<float>(std::uint32_t{0xc08d3b9du})), seed(initial_seed) {
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

void VcfSampleState::reset(float resonance) noexcept {
    if (resonance < 0.98f) s1 = s2 = delayed_bp = 0.0f;
    else {
        const float value = (resonance - 0.98f) * 10.5f;
        if (mode == VcfOutput::bandpass) {
            s1 = delayed_bp = 0.0f;
            s2 = value;
        } else {
            s1 = delayed_bp = value;
            s2 = 0.0f;
        }
    }
    feedback = dc_state = 0.0f;
}

float VcfSampleState::process_sample(float input, float g, float r) noexcept {
    constexpr float a = vcf_constant<0x180559ec4>();
    constexpr float b = vcf_constant<0x180559da0>();
    constexpr float k = vcf_constant<0x18055a4c8>();
        float sample = input;
        if (noise) {
            seed ^= seed << 13;
            seed ^= seed >> 17;
            seed ^= seed << 5;
            sample = (static_cast<float>(seed) * vcf_constant<0x180554834>() + sample) - vcf_constant<0x1805587e8>();
        }
        const float numerator = ((((r * k - (g * a + b)) * s1 + sample * vcf_constant<0x180559e8c>()) - s2 * a) + (s1 - delayed_bp) * compensation) + feedback * k;
        const float denominator = (((g * g) * a + g * b) + a) - ((g * k) * r) - compensation * g;
        const float hp = numerator / denominator;
        const float g_hp = hp * g;
        const float bp = g_hp + s1;
        const float g_bp = bp * g;
        const float lp = g_bp + s2;
        s1 = bp + g_hp;
        s2 = lp + g_bp;
        delayed_bp = bp;
        const float drive = bp * vcf_constant<0x180559f68>() - ((hp + lp) * a + bp * b);
        const float x = std::abs(drive) * vcf_constant<0x18055a2c4>() + offset;
        const float sign = std::bit_cast<float>((std::bit_cast<std::uint32_t>(drive) & 0xbf800000u) | 0x3f800000u);
        feedback = (feedback_curve(x) * vcf_constant<0x180554a1c>() - vcf_constant<0x180554890>()) * sign;
        const float selected = mode == VcfOutput::lowpass ? lp : mode == VcfOutput::bandpass ? bp : mode == VcfOutput::highpass ? hp : 0.0f;
        const float raw = (saturate(vcf_constant<0x180559c34>() - selected * vcf_constant<0x180559e28>()) - baseline) * vcf_constant<0x18055a1e8>();
        const float y = raw * feedforward + dc_state;
        dc_state = y * dc_coefficient + raw * negative_feedforward;
        return y * vcf_constant<0x18055a048>();
}

}  // namespace mfrk::detail
