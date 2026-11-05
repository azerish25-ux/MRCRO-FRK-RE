#pragma once
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
namespace mfrk::detail::ucrt_pitch_powf {
// Fixed-base2 slice of source UCRT FMA powf, valid finite exponent[-5.75, float32(58*float32(1/12))].
// Nonfinite/other bases, errno/fenv, and unrelated powf ranges are outside this profile.
constexpr double d(std::uint64_t x) noexcept{return std::bit_cast<double>(x);}
inline constexpr std::array<std::uint64_t,64> exp_table={0x3ff0000000000000ull,0x3ff02c9a3e778061ull,0x3ff059b0d3158574ull,0x3ff0874518759bc8ull,0x3ff0b5586cf9890full,0x3ff0e3ec32d3d1a2ull,0x3ff11301d0125b51ull,0x3ff1429aaea92de0ull,0x3ff172b83c7d517bull,0x3ff1a35beb6fcb75ull,0x3ff1d4873168b9aaull,0x3ff2063b88628cd6ull,0x3ff2387a6e756238ull,0x3ff26b4565e27cddull,0x3ff29e9df51fdee1ull,0x3ff2d285a6e4030bull,0x3ff306fe0a31b715ull,0x3ff33c08b26416ffull,0x3ff371a7373aa9cbull,0x3ff3a7db34e59ff7ull,0x3ff3dea64c123422ull,0x3ff4160a21f72e2aull,0x3ff44e086061892dull,0x3ff486a2b5c13cd0ull,0x3ff4bfdad5362a27ull,0x3ff4f9b2769d2ca7ull,0x3ff5342b569d4f82ull,0x3ff56f4736b527daull,0x3ff5ab07dd485429ull,0x3ff5e76f15ad2148ull,0x3ff6247eb03a5585ull,0x3ff6623882552225ull,0x3ff6a09e667f3bcdull,0x3ff6dfb23c651a2full,0x3ff71f75e8ec5f74ull,0x3ff75feb564267c9ull,0x3ff7a11473eb0187ull,0x3ff7e2f336cf4e62ull,0x3ff82589994cce13ull,0x3ff868d99b4492edull,0x3ff8ace5422aa0dbull,0x3ff8f1ae99157736ull,0x3ff93737b0cdc5e5ull,0x3ff97d829fde4e50ull,0x3ff9c49182a3f090ull,0x3ffa0c667b5de565ull,0x3ffa5503b23e255dull,0x3ffa9e6b5579fdbfull,0x3ffae89f995ad3adull,0x3ffb33a2b84f15fbull,0x3ffb7f76f2fb5e47ull,0x3ffbcc1e904bc1d2ull,0x3ffc199bdd85529cull,0x3ffc67f12e57d14bull,0x3ffcb720dcef9069ull,0x3ffd072d4a07897cull,0x3ffd5818dcfba487ull,0x3ffda9e603db3285ull,0x3ffdfc97337b9b5full,0x3ffe502ee78b3ff6ull,0x3ffea4afa2a490daull,0x3ffefa1bee615a27ull,0x3fff50765b6e4540ull,0x3fffa7c1819e90d8ull};
inline bool admitted(float y)noexcept{return y>=-5.75f && y<=std::bit_cast<float>(0x409aaaabu);}
inline std::int32_t nearest_even(double x)noexcept{
 auto n=static_cast<std::int32_t>(x);const double f=x-static_cast<double>(n);
 if(f>.5||(f==.5&&(n%2)!=0))++n;
 else if(f<-.5||(f==-.5&&(n%2)!=0))--n;
 return n;
}
inline float evaluate(float y)noexcept{
 if(y==0.0f)return 1.0f;
 if(y==1.0f)return 2.0f;
 const double t=static_cast<double>(y)*d(0x3fe62e42fefa39efull);
 const auto n=nearest_even(t*d(0x40571547652b82feull));
 const double r=std::fma(-static_cast<double>(n),d(0x3f862e42fefa39efull),t);
 double p=std::fma(d(0x3fc5555555555555ull),r,0.5);
 p=std::fma(p,r,1.0);p=r*p;
 const auto index=static_cast<std::uint32_t>(n)&63u;
 const double table=d(exp_table[index]);const double fraction=std::fma(p,table,table);
 const auto scale=n>=0?n/64:-((-n+63)/64);
 const auto bits=std::bit_cast<std::uint64_t>(fraction)+(static_cast<std::uint64_t>(scale)<<52);
 return static_cast<float>(d(bits));
}
inline bool try_evaluate(float y,float& output)noexcept{if(!admitted(y))return false;output=evaluate(y);return true;}
}
