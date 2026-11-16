#pragma once
#include "ui_master_power.hpp"
#include "ui_exponential_tables.hpp"
#include <limits>
namespace mfrk::ui::detail {
inline float exponential_float(std::uint32_t bits){return std::bit_cast<float>(bits);}
// Single-rounding finite binary32 FMA. An exact binary64 product and TwoSum
// residue correct a possible double-rounding midpoint. This avoids dependence
// on platform float-FMA fallbacks. Finite operands/result, RN-even and gradual
// underflow; implicit contraction must stay disabled. FP flags are not modeled.
inline float exponential_fma(float x,float y,float z){
 const double a=static_cast<double>(x)*static_cast<double>(y),b=z,sum=a+b;
 const double virtual_b=sum-a,error=(a-(sum-virtual_b))+(b-virtual_b);
 const float rounded=static_cast<float>(sum);const double rounded_double=rounded;
 if((error>0&&rounded_double<sum)||(error<0&&rounded_double>sum)){
  const float adjacent=std::nextafter(rounded,error>0?std::numeric_limits<float>::infinity():-std::numeric_limits<float>::infinity());
  if(sum==(rounded_double+static_cast<double>(adjacent))*.5)return adjacent;
 }
 return rounded;
}
// Pinned UCRT10.0.26100.9711 FMA-dispatch expf value path, input0..<88.
inline float exponential_exp(float x){
 const double xd=x;const int n=master_nearest_even(xd*master_double(0x40571547652b82feull));
 const unsigned index=static_cast<unsigned>(n)&63;
 const double r=std::fma(-static_cast<double>(n),master_double(0x3f862e42fefa39efull),xd);
 const double squared=r*r;
 double p=std::fma(master_double(0x3fc5555555555555ull),r,.5);
 p=std::fma(squared,p,r);
 const double t=master_table(master_exp_fma_bits,index);p=std::fma(p,t,t);
 const double scale=master_double(static_cast<std::uint64_t>((n-static_cast<int>(index))/64+1023)<<52);
 return static_cast<float>(p*scale);
}
// Same runtime's logf value path for finite inputs>=1. Source floating-point
// ordering and its distinct near-one polynomial remain explicit.
inline float exponential_log(float x){
 const float t=x-1.f;
 if(std::abs(t)<.0625f){
  const float v=t/(t+2.f),correction=t*v,s=v+v,squared=s*s,cubed=s*squared;
  float p=exponential_fma(squared,exponential_float(0x3c4ccccdu),exponential_float(0x3daaaaabu));
  p=exponential_fma(p,cubed,-correction);return t+p;
 }
 const std::uint32_t bits=std::bit_cast<std::uint32_t>(x);
 const std::uint32_t mantissa=bits&0x007fffffu;
 const float exponent=static_cast<float>(static_cast<int>(bits>>23)-127);
 const unsigned index=(mantissa>>16)+((mantissa>>15)&1);
 const float center=exponential_float((index<<16)|0x3f000000u),normalized=exponential_float(mantissa|0x3f000000u);
 float r=(center-normalized)*exponential_float(exponential_log_reciprocal_bits[index]);
 const float p=exponential_fma(exponential_float(0x3eaaaaabu),r,.5f),squared=r*r;
 r=exponential_fma(p,squared,r);
 float tail=exponential_fma(exponential_float(0x3805fdf4u),exponent,-r);
 const float lead=exponential_fma(exponential_float(0x3f317000u),exponent,exponential_float(exponential_log_lead_bits[index]));
 tail=tail+exponential_float(exponential_log_tail_bits[index]);return lead+tail;
}
}
