#pragma once

#include <array>
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
inline constexpr std::uint32_t finite_upper_abs_bits_exclusive=0x7f800000u;
[[nodiscard]] inline bool in_domain(float value) noexcept {
    return (std::bit_cast<std::uint32_t>(value) & 0x7fffffffu) <
           finite_upper_abs_bits_exclusive;
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
// Source inline reducer RVA0xac490..0xac5e4. Only bytes119..154 of the
// fixed-point2/pi table are addressed by a finite binary32 input.
inline constexpr std::array<std::uint8_t,36> two_over_pi_window = {
    0x15,0xef,0x7a,0xaf,0x8e,0x45,0xf9,0x07,0x41,0x0e,0xf1,0x64,
    0x56,0x8a,0x6d,0x03,0x77,0xd3,0xd4,0x47,0x5f,0x9d,0xf0,0xa7,
    0x54,0x10,0x39,0xb9,0x0d,0xe6,0x8b,0x02,0x00,0x00,0x00,0x00
};
struct Wide { std::uint64_t low, high; };
inline Wide multiply(std::uint64_t a, std::uint64_t b) noexcept {
    constexpr std::uint64_t mask=0xffffffffu;
    const auto a0=a&mask, a1=a>>32, b0=b&mask, b1=b>>32;
    const auto w0=a0*b0, t=a1*b0+(w0>>32);
    auto w1=t&mask;
    const auto w2=t>>32;
    w1+=a0*b1;
    return {(w1<<32)|(w0&mask), a1*b1+w2+(w1>>32)};
}
inline std::uint64_t load_word(unsigned offset) noexcept {
    std::uint64_t x=0;
    for(unsigned i=0;i<8;++i)
        x|=std::uint64_t{two_over_pi_window[offset+i]}<<(8*i);
    return x;
}
struct Reduction { double remainder; unsigned quadrant; };
inline Reduction large_reduce(double value) noexcept {
    const auto bits=std::bit_cast<std::uint64_t>(value);
    const unsigned exponent=static_cast<unsigned>(bits>>52)-1023u;
    // Finite large binary32 inputs have exponent24..127: offset0..12.
    const unsigned offset=134u-(exponent>>3)-119u;
    const auto mantissa=(bits&0xfffffffffffffull)|0x10000000000000ull;
    const auto p0=multiply(load_word(offset),mantissa);
    const auto p1=multiply(load_word(offset+8),mantissa);
    const auto p2=multiply(load_word(offset+16),mantissa);
    auto low=p0.low, middle=p1.low+p0.high;
    auto high=p1.high+std::uint64_t{middle<p1.low}+p2.low;
    const unsigned residue=exponent&7u, shift=54u-residue;
    auto quotient=high>>shift;
    const auto carry=(high>>(shift-1u))&1u;
    std::uint64_t sign=0;
    if(carry) {
        // Original uses one's complement, without adding one.
        high=~high;
        middle=~middle;
        low=~low;
        sign=0x8000000000000000ull;
    }
    quotient=(quotient+carry)&3u;
    high&=(std::uint64_t{1}<<(54u-residue))-1u;
    int base=static_cast<int>(residue)-54;
    if(high==0) {
        high=middle;
        middle=low;
        low=0;
        base-=64;
    }
    // Exhaustive finite binary32 integer-reduction check excludes zero.
    assert(high!=0);
    const int top=63-std::countl_zero(high);
    base+=top;
    const int align=top-52;
    if(align>0) {
        const auto saved=high;
        high>>=static_cast<unsigned>(align);
        middle>>=static_cast<unsigned>(align);
        middle|=saved<<static_cast<unsigned>(64-align);
    } else if(align<0) {
        const auto saved=middle;
        const auto left=static_cast<unsigned>(-align);
        high<<=left;
        middle<<=left;
        high|=saved>>(64u-left);
        low>>=(64u-left);
        middle|=low;
    }
    const auto result_bits=(high&~0x10000000000000ull)|sign|
                          (static_cast<std::uint64_t>(base+1023)<<52);
    return {std::bit_cast<double>(result_bits)*d(0x3ff921fb54442d18ull),
            static_cast<unsigned>(quotient)};
}

}

// Preconditions: IEEE binary32/64, round-to-nearest-even, finite input with
// absbits<0x7f800000 (all finite binary32). Nonfinite paths and errno/fenv
// exception-side-effect compatibility are outside this source profile.
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
    double reduced;
    unsigned multiple;
    if(absolute_bits>=ordinary_upper_abs_bits_exclusive) {
        const auto r=detail::large_reduce(x);
        reduced=r.remainder;
        multiple=r.quadrant;
    } else {
        const double nearest=detail::fused(x,detail::d(0x3fe45f306dc9c883ull),0.5);
        multiple=static_cast<unsigned>(nearest);
        const double n=static_cast<double>(multiple);
        const double high=detail::fused(-n,detail::d(0x3ff921fb54400000ull),x);
        const double low=n*detail::d(0x3dd0b4611a626331ull);
        reduced=high-low;
    }
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

