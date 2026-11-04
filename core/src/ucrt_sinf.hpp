#pragma once

#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

// Return-bit compatibility with the identified UCRT AVX/FMA scalar sinf branch.
// Source DLL SHA256 dedb7611e1abc1ff3852dcac3d4c93a048d729e25b061973b9969f22698f14ca,
// export RVA0xac360, nonzero dispatcher flag atRVA0x139e60 (reference observed3).
// This is NOT a host-libm sin wrapper or fitted approximation.
namespace mfrk::detail::ucrt_sinf {

inline constexpr std::uint32_t ordinary_upper_abs_bits_exclusive = 0x4b800456u;
[[nodiscard]] inline bool in_domain(float value) noexcept {
    return (std::bit_cast<std::uint32_t>(value) & 0x7fffffffu) <
           ordinary_upper_abs_bits_exclusive;
}

namespace detail {
static_assert(sizeof(float)==4 && sizeof(double)==8 &&
              std::numeric_limits<float>::is_iec559 &&
              std::numeric_limits<double>::is_iec559);
constexpr double d(std::uint64_t bits) noexcept { return std::bit_cast<double>(bits); }
inline double fused(double a,double b,double c) noexcept { return std::fma(a,b,c); }
inline double sine_polynomial(double x) noexcept {
    const double z=x*x;
    double polynomial=fused(z,d(0x3ec71de3a556c734ull),d(0xbf2a01a01a01a01aull));
    polynomial=fused(polynomial,z,d(0x3f81111111111111ull));
    polynomial=fused(polynomial,z,d(0xbfc5555555555555ull));
    const double cubic=x*z;
    return fused(polynomial,cubic,x);
}
inline double cosine_polynomial(double x) noexcept {
    const double z=x*x;
    const double base=fused(z,d(0xbfe0000000000000ull),1.0);
    double polynomial=fused(z,d(0xbe927e4fb7789f5cull),d(0x3efa01a01a01a019ull));
    polynomial=fused(polynomial,z,d(0xbf56c16c16c16c16ull));
    polynomial=fused(polynomial,z,d(0x3fa5555555555555ull));
    const double fourth=z*z;
    return fused(polynomial,fourth,base);
}
}

// Preconditions: IEEE binary32/64, round-to-nearest-even, finite input with
// absbits<0x4b800456. Covers all valid FX angles0..float(pi), angles[-64,+64] and original
// Harmo constructor arguments. Large Payne-Hanek/nonfinite paths and errno/fenv
// exception-side-effect compatibility are outside this candidate; no silent fallback.
// Keep global fast-math/contraction disabled; these explicit double FMAs are required.
[[nodiscard]] inline float evaluate(float value) noexcept {
    assert(in_domain(value));
    const auto input_bits=std::bit_cast<std::uint32_t>(value);
    const auto absolute_bits=input_bits & 0x7fffffffu;
    if(absolute_bits<=0x3f490fdbu) {
        if(absolute_bits<0x39000000u) return value; // Includes exact signed zero/subnormals.
        const double x=static_cast<double>(value);
        if(absolute_bits<0x3c000000u) {
            const double square=x*x;
            const double cubic=square*x;
            return static_cast<float>(detail::fused(-cubic,detail::d(0x3fc5555555555555ull),x));
        }
        return static_cast<float>(detail::sine_polynomial(x));
    }
    const double x=static_cast<double>(std::bit_cast<float>(absolute_bits));
    const double nearest=detail::fused(x,detail::d(0x3fe45f306dc9c883ull),0.5);
    const auto multiple=static_cast<std::int32_t>(nearest); // Positive ordinary-domain truncation.
    const double n=static_cast<double>(multiple);
    const double high=detail::fused(-n,detail::d(0x3ff921fb54400000ull),x);
    const double low=n*detail::d(0x3dd0b4611a626331ull);
    const double reduced=high-low;
    double result=(multiple&1) ? detail::cosine_polynomial(reduced)
                               : detail::sine_polynomial(reduced);
    // Original XOR sign masks: quadrant2/3 and original input sign.
    std::uint64_t sign=0;
    if(multiple&2) sign^=0x8000000000000000ull;
    if(input_bits&0x80000000u) sign^=0x8000000000000000ull;
    result=std::bit_cast<double>(std::bit_cast<std::uint64_t>(result)^sign);
    return static_cast<float>(result);
}

} // namespace mfrk::detail::ucrt_sinf

