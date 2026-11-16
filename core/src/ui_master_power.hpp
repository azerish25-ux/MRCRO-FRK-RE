#pragma once
#include "ui_master_power_tables.hpp"
#include <bit>
#include <cmath>
namespace mfrk::ui::detail {
inline double master_double(std::uint64_t bits){return std::bit_cast<double>(bits);}
inline double master_table(const auto& bits,unsigned i){return master_double(bits[i]);}
inline int master_nearest_even(double value){
 const double lower_value=std::floor(value);const auto lower=static_cast<int>(lower_value);
 const double fraction=value-lower_value;
 return lower+(fraction>.5||(fraction==.5&&(lower&1)));
}
// Reference UCRT10.0.26100.9711 powf, positive FMA-dispatch value path for
// x in0..1 and exponents2/.5 only. Round-to-nearest-even, finite intermediates,
// gradual underflow and FP contraction disabled except explicit fma. This
// reproduces the source value policy, not errno or floating exception flags.
inline float master_power(float x,bool inverse){
 if(x==0.f)return 0.f;
 const double xd=x;double exponent;
 const bool near_one=std::abs(xd-1.)<.0625;
 const double y=inverse?.5:2.;
 if(near_one){
  // Even with FMA dispatch selected, the source's near-one path is unfused
  // and jumps into its separate SSE exponential polynomial.
  const double t=xd-1.,v=t/(t+2.),s=v+v,z=s*s;
  const double low=z*master_double(0x3f89999999bac6d4ull)+master_double(0x3fb55555555554e6ull);
  const double high=z*master_double(0x3f3c8034c85dfff0ull)+master_double(0x3f62492307f1519full);
  const double a=s*z,b=(z*z)*a;
  const double logarithm=t+((b*high+a*low)-v*t);
  exponent=y*logarithm;
 }else{
  const std::uint64_t xb=std::bit_cast<std::uint64_t>(xd);
  const std::uint64_t mantissa=xb&0x000fffffffffffffull;
  const auto index=static_cast<unsigned>((mantissa>>44)+((mantissa>>43)&1));
  const double center=master_double((std::uint64_t(index)|0x3fe00ull)<<44);
  const double normalized=master_double(mantissa|0x3fe0000000000000ull);
  double r=(center-normalized)*master_table(master_reciprocal_bits,index);
  double p=std::fma(master_double(0x3fd5555555555555ull),r,.5);
  p=std::fma(p,r,1.);r=r*p;
  const double e=static_cast<double>(static_cast<int>(xb>>52)-1023);
  exponent=y*((e*master_double(0x3fe62e42fefa39efull)+master_table(master_log_bits,index))-r);
 }
 if(exponent<=master_double(0xc059d1da00000000ull))return 0.f;
 const int n=master_nearest_even(exponent*master_double(0x40571547652b82feull));
 const double nf=n,step=master_double(0x3f862e42fefa39efull);
 const double t=master_table(master_exp_fma_bits,static_cast<unsigned>(n)&63);
 double value;
 if(near_one){
  const double r=exponent-nf*step;
  double p=master_double(0x3fc5555555555555ull)*r;
  p=p+.5;p=(r*r)*p;p=p+r;
  value=p*t+t;
 }else{
  const double r=std::fma(-nf,step,exponent);
  double p=std::fma(master_double(0x3fc5555555555555ull),r,.5);
  p=std::fma(p,r,1.);p=r*p;
  value=std::fma(p,t,t);
 }
 const auto scaled=std::bit_cast<std::uint64_t>(value)+
  (static_cast<std::uint64_t>(static_cast<std::int64_t>(n>>6))<<52);
 return static_cast<float>(master_double(scaled));
}
}
