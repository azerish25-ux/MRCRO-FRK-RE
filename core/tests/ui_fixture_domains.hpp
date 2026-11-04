#pragma once
#include "mfrk/ui.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace mfrk::ui::test {
inline void require(bool condition,const char*message){if(!condition)throw std::runtime_error(message);}
inline bool finite(float x)noexcept{return (std::bit_cast<std::uint32_t>(x)&0x7f800000u)!=0x7f800000u;}
inline bool finite(double x)noexcept{return (std::bit_cast<std::uint64_t>(x)&0x7ff0000000000000ull)!=0x7ff0000000000000ull;}
// These are conservative test-fixture admission checks, not additional public API
// behavior. Identity authentication has already happened before payload decoding.
inline void ordinary(float x){require(finite(x)&&std::abs(x)<=1000000.f,"fixture finite ordinary value");}
inline void dimensions(std::int32_t w,std::int32_t h){require(w>=0&&h>=0,"fixture nonnegative dimensions");}
inline void trig_argument(float x){require(finite(x)&&std::abs(x)<=64.f,"fixture packed-trig domain");}
inline void center_domain(std::int32_t w,std::int32_t h,const std::array<float,5>&s){dimensions(w,h);for(float x:s)ordinary(x);}
inline void gradient_domain(std::int32_t w,std::int32_t h,const std::array<float,3>&s){dimensions(w,h);for(float x:s)ordinary(x);const float total=s[0]+s[1],midpoint=total*.5f,base=midpoint-s[1],theta=s[2]*std::bit_cast<float>(0x40490fdbu)+base,opposite=theta+std::bit_cast<float>(0x40490fdbu);for(float angle:{theta,opposite,theta+std::bit_cast<float>(0x3fc90fdbu),opposite+std::bit_cast<float>(0x3fc90fdbu)})trig_argument(angle);}
inline void marker_domain(std::int32_t w,std::int32_t h,const std::array<float,4>&s){dimensions(w,h);for(float x:s)ordinary(x);require(s[1]==0.f||s[1]==1.f,"fixture reverse boolean");}
inline void magnetic_domain(std::span<const MagneticZone>zones,float value,float delta){ordinary(value);ordinary(delta);float previous_position=0,previous_value=0;bool first=true;for(const auto&z:zones){for(float x:{z.position_from,z.position_to,z.value_from,z.value_to})require(finite(x)&&x>=0&&x<=1,"fixture magnetic endpoints");require(z.position_from<=z.position_to&&z.value_from<=z.value_to,"fixture zone direction");if(!first)require(z.position_from>=previous_position&&z.value_from>=previous_value,"fixture zone ordering");const float p=z.position_to-z.position_from,v=z.value_to-z.value_from;require(p==0||p>=1e-20f,"fixture magnetic position span");require(v==0||v>=1e-20f,"fixture magnetic value span");previous_position=z.position_to;previous_value=z.value_to;first=false;}}
inline void flex_domain(const std::array<float,4>&v){for(float x:v)require(finite(x),"fixture finite Flex input");const float right=v[0]+v[2],bottom=v[1]+v[3];for(float x:{v[0],v[1],right,bottom})require(finite(x)&&double(x)>=-2147483648.&&double(x)<2147483648.,"fixture signed32 edge conversion");const double width=std::trunc(double(right))-std::trunc(double(v[0])),height=std::trunc(double(bottom))-std::trunc(double(v[1]));require(width>=-2147483648.&&width<=2147483647.&&height>=-2147483648.&&height<=2147483647.,"fixture signed32 edge differences");}
inline void scaled_domain(float scale,double logical){require(finite(scale)&&scale>=0&&finite(logical)&&logical>=0,"fixture scaled-dimension input");const double product=double(scale)*logical,shifted=product+.5;require(finite(product)&&finite(shifted)&&shifted>=0&&shifted<2147483648.,"fixture signed32 dimension conversion");}
} // namespace mfrk::ui::test
