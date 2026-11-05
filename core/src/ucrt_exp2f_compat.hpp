#pragma once
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
// UCRT SHA256 dedb7611e1abc1ff3852dcac3d4c93a048d729e25b061973b9969f22698f14ca.
// Export RVA0x79a70, dispatcher&3==3 AVX path.
// Admitted return-bit profile finite exponents[-64,+64], RN-even;
// no errno/fenv/special/overflow/underflow/SSE backend compatibility.
// Source uses separate double multiply/add, NOT FMA. Fastmath/contraction off.
namespace mfrk::detail::ucrt_exp2f {
constexpr double d(std::uint64_t bits){return std::bit_cast<double>(bits);}
inline constexpr std::array<std::uint64_t,32> table={0x3ff0000000000000ull,0x3fefd9b0d3158574ull,0x3fefb5586cf9890full,0x3fef9301d0125b51ull,0x3fef72b83c7d517bull,0x3fef54873168b9aaull,0x3fef387a6e756238ull,0x3fef1e9df51fdee1ull,0x3fef06fe0a31b715ull,0x3feef1a7373aa9cbull,0x3feedea64c123422ull,0x3feece086061892dull,0x3feebfdad5362a27ull,0x3feeb42b569d4f82ull,0x3feeab07dd485429ull,0x3feea47eb03a5585ull,0x3feea09e667f3bcdull,0x3fee9f75e8ec5f74ull,0x3feea11473eb0187ull,0x3feea589994cce13ull,0x3feeace5422aa0dbull,0x3feeb737b0cdc5e5ull,0x3feec49182a3f090ull,0x3feed503b23e255dull,0x3feee89f995ad3adull,0x3feeff76f2fb5e47ull,0x3fef199bdd85529cull,0x3fef3720dcef9069ull,0x3fef5818dcfba487ull,0x3fef7c97337b9b5full,0x3fefa4afa2a490daull,0x3fefd0765b6e4540ull};
inline float evaluate(float input){assert(std::isfinite(input)&&input>=-64&&input<=64);
 const double x=input;const double rounded=x+d(0x42e8000000000000ull);const double center=rounded-d(0x42e8000000000000ull);const double residual=x-center;
 const double linear=residual*d(0x3fe62e42ff0c52d6ull);const double poly1=residual*d(0x3fac6af84b912394ull);const double poly2=poly1+d(0x3fcebfce50fac4f3ull);const double square=residual*residual;const double cubic=poly2*square;const double base=linear+1.0;const double polynomial=cubic+base;
 const auto n=std::bit_cast<std::uint64_t>(rounded);const auto scale_bits=(n<<47)+table[n&31];const double scale=std::bit_cast<double>(scale_bits);const double result=polynomial*scale;return static_cast<float>(result);
}
}
