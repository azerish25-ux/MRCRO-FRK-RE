#include "mfrk/ui.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <limits>
static_assert(sizeof(float)==4 && std::numeric_limits<float>::is_iec559);
static_assert(sizeof(double)==8 && std::numeric_limits<double>::is_iec559);
namespace mfrk::ui {
namespace {
namespace native_packed_trig {
inline constexpr float value(std::uint32_t bits) noexcept { return std::bit_cast<float>(bits); }
inline float lane(float x) noexcept {
    const auto input=std::bit_cast<std::uint32_t>(x);
    const float magnitude=value(input&0x7fffffffu);
    const float scaled=magnitude*value(0x3ea2f983u);
    const float rounded=scaled+value(0x4b400000u);
    const float multiple=rounded-value(0x4b400000u);
    const auto parity=std::bit_cast<std::uint32_t>(rounded)<<31;
    const float high=multiple*value(0x40490000u);
    float reduced=magnitude-high;
    const float middle=multiple*value(0x3a7da000u); reduced=reduced-middle;
    const float small=multiple*value(0x34222000u); reduced=reduced-small;
    const float low=multiple*value(0x2cb4611au); reduced=reduced-low;
    const float square=reduced*reduced;
    const float signed_reduced=value(std::bit_cast<std::uint32_t>(reduced)^parity);
    float polynomial=value(0x362edef8u)*square;
    polynomial=polynomial+value(0xb94fb7ffu);
    polynomial=polynomial*square;polynomial=polynomial+value(0x3c088766u);
    polynomial=polynomial*square;polynomial=polynomial+value(0xbe2aaaa6u);
    float correction=square*polynomial;correction=correction*signed_reduced;
    const float result=signed_reduced+correction;
    return value(std::bit_cast<std::uint32_t>(result)^(input&0x80000000u));
}
}
using native_packed_trig::value;
using native_packed_trig::lane;
inline const MagneticZone* zone_for_value(std::span<const MagneticZone> zones,float value) noexcept {
 for(const auto& z:zones)if(z.value_from==value||(z.value_from<value&&value<z.value_to))return &z;
 const MagneticZone* previous=nullptr;
 for(const auto& z:zones){
  if(z.value_from<value){previous=&z;continue;}
  if(previous&&std::abs(value-previous->value_to)<=std::abs(z.value_from-value))return previous;
  return &z;
 }
 return previous;
}
}
std::array<float,4> interpolated_center_bounds(std::int32_t width,
    std::int32_t height,float normal_radius,float hover_radius,
    float normal_center_size,float hover_center_size,float transition) noexcept {
    const float remaining=1.0f-transition;
    const float radius_base=remaining*normal_radius;
    const float radius_hover=transition*hover_radius;
    const float radius=radius_base+radius_hover;
    const float center_base=remaining*normal_center_size;
    const float center_hover=transition*hover_center_size;
    const float center_size=center_base+center_hover;
    const float integer_half=static_cast<float>(std::min(width/2,height/2));
    const float outer_radius=radius*integer_half;
    const float center_radius=center_size*outer_radius;
    const float center_x=static_cast<float>(width)*0.5f;
    const float center_y=static_cast<float>(height)*0.5f;
    const float diameter=center_radius+center_radius;
    return {center_x-center_radius,center_y-center_radius,diameter,diameter};
}

std::array<float,2> packed_sine_cosine(float x) noexcept {
    const auto magnitude=std::bit_cast<std::uint32_t>(x)&0x7fffffffu;
    assert(magnitude<=0x42800000u);
    if(magnitude<0x39000000u) {
        float square=value(magnitude)*value(magnitude);
        square=square*0.5f;
        return {x,1.0f-square};
    }
    const float shifted=value(magnitude)+value(0x3fc90fdbu);
    return {lane(x),lane(shifted)};
}

std::array<float,4> outline_gradient_points(std::int32_t width,
    std::int32_t height,float right_angle,float left_angle,
    float gradient_angle,float center_radius) noexcept {
    const float total=left_angle+right_angle;
    const float midpoint=total*0.5f;
    const float base=midpoint-left_angle;
    const float offset=gradient_angle*native_packed_trig::value(0x40490fdbu);
    const float theta=offset+base;
    const float opposite=theta+native_packed_trig::value(0x40490fdbu);
    const float first_cosine_argument=theta+native_packed_trig::value(0x3fc90fdbu);
    const float other_cosine_argument=opposite+native_packed_trig::value(0x3fc90fdbu);
    const float first_sine=native_packed_trig::lane(theta);
    const float first_cosine=native_packed_trig::lane(first_cosine_argument);
    const float other_sine=native_packed_trig::lane(opposite);
    const float other_cosine=native_packed_trig::lane(other_cosine_argument);
    const float center_x=static_cast<float>(width)*0.5f;
    const float center_y=static_cast<float>(height)*0.5f;
    return {first_sine*center_radius+center_x,center_y-first_cosine*center_radius,
            other_sine*center_radius+center_x,center_y-other_cosine*center_radius};
}

std::array<float,4> center_marker_points(std::int32_t width,std::int32_t height,
    float parameter,bool reverse,float right_angle,float left_angle,
    float center_radius,float start,float marker_width) noexcept {
    float position=reverse?1.0f-parameter:parameter;
    position=std::clamp(position,0.0f,1.0f);
    const float radius=center_radius*native_packed_trig::value(0x3f866666u);
    const float remaining=1.0f-start;
    const float distance=remaining*radius;
    const float signed_distance=native_packed_trig::value(std::bit_cast<std::uint32_t>(distance)^0x80000000u);
    const float total=left_angle+right_angle;
    const float theta=total*position-left_angle;
    const float sine=native_packed_trig::lane(theta);
    const float cosine=native_packed_trig::lane(theta+native_packed_trig::value(0x3fc90fdbu));
    const float negative_sine=native_packed_trig::value(std::bit_cast<std::uint32_t>(sine)^0x80000000u);
    const float center_x=static_cast<float>(width)*0.5f;
    const float center_y=static_cast<float>(height)*0.5f;
    const float end=(marker_width-remaining)*radius;
    return {signed_distance*negative_sine+center_x,signed_distance*cosine+center_y,
            center_x-sine*end,end*cosine+center_y};
}

MagneticResult magnetic_delta(std::span<const MagneticZone> zones,
    float value,float delta,bool flag20,bool flag21) noexcept {
 const auto* before=zone_for_value(zones,value);float position=value;
 if(before){
  const auto& z=*before;
  if(z.position_from==z.position_to)position=z.position_from;
  else if(z.value_from==z.value_to){
   if((flag20||flag21)&&z.skip_at_drag_start)position=delta<=0.0f?z.position_from:z.position_to;
   else position=(z.position_to+z.position_from)*0.5f;
  }else{
   const float distance=value-z.value_from;
   const float span=z.value_to-z.value_from;
   const float ratio=distance/span;
   const float output_span=z.position_to-z.position_from;
   position=ratio*output_span+z.position_from;
  }
 }
 position=position+delta;float result=position;
 for(const auto& z:zones){
  if(z.position_from==position||(z.position_from<position&&position<z.position_to)){
   if(z.position_from!=z.position_to&&z.value_from!=z.value_to){
    const float distance=position-z.position_from;
    const float span=z.position_to-z.position_from;
    const float ratio=distance/span;
    const float output_span=z.value_to-z.value_from;
    result=ratio*output_span+z.value_from;
   }else result=z.value_from;
   break;
  }
 }
 return {result,before&&before->skip_at_drag_start&&flag21};
}

std::array<std::int32_t,4> quantize_flex_edges(float x,float y,float width,float height) noexcept {
 const float right=x+width,bottom=y+height;
 const auto left_i=static_cast<std::int32_t>(x),top_i=static_cast<std::int32_t>(y);
 return {left_i,top_i,static_cast<std::int32_t>(right)-left_i,static_cast<std::int32_t>(bottom)-top_i};
}

std::int32_t scaled_absolute_dimension(float view_scale,double logical_size) noexcept {
 const double product=static_cast<double>(view_scale)*logical_size;
 const double shifted=product+0.5;
 return static_cast<std::int32_t>(shifted);
}

} // namespace mfrk::ui
