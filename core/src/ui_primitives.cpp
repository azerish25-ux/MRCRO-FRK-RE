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

std::array<std::int32_t,4> knob_modulation_dirty_bounds(std::int32_t width,
    std::int32_t height,float right_angle,float left_angle,float radius,
    float thickness,const std::array<float,5>& previous,
    const std::array<float,5>& current) noexcept {
    const float half=static_cast<float>(std::min(width/2,height/2));
    const float outer=radius*half;
    const float ring_radius=(1.f-thickness*.5f)*outer;
    const float stroke_half=(thickness*outer)*.5f;
    const float total=left_angle+right_angle;
    const float inverse=1.f/total;
    const float pi_half=value(0x3fc90fdbu);
    const std::array<float,3> crossings{(left_angle-pi_half)*inverse,.5f,(left_angle+pi_half)*inverse};
    const float center_x=static_cast<float>(width)*.5f,center_y=static_cast<float>(height)*.5f;
    const float negative_radius=-ring_radius;
    std::array<std::array<float,2>,3> extrema{};
    // Source dd423 computes both first angles together with its packed lane
    // path, including the midpoint's cosine lane (no scalar tiny shortcut).
    for(unsigned i=0;i<2;++i){
        const float angle=crossings[i]*total-left_angle;
        extrema[i]={(-lane(angle))*negative_radius+center_x,
                    lane(angle+pi_half)*negative_radius+center_y};
    }
    const auto last=packed_sine_cosine(crossings[2]*total-left_angle);
    extrema[2]={(-last[0])*negative_radius+center_x,last[1]*negative_radius+center_y};
    const auto one=[&](const std::array<float,5>& statistics){
        const auto point=[&](float position){
            position=std::clamp(position,0.f,1.f);
            const auto trig=packed_sine_cosine(total*position-left_angle);
            return std::array<float,2>{(-trig[0])*negative_radius+center_x,
                                      trig[1]*negative_radius+center_y};
        };
        const auto first=point(statistics[0]),second=point(statistics[1]);
        float x=std::min(second[0],first[0]),y=std::min(second[1],first[1]);
        float w=first[0]-second[0],h=first[1]-second[1];
        if(w<0.f)w=-w;
        if(h<0.f)h=second[1]-first[1];
        for(unsigned i=0;i<3;++i)if(statistics[0]<crossings[i]&&crossings[i]<statistics[1]){
            const auto p=extrema[i];
            if(w>0.f&&h>0.f){
                const float nx=std::min(x,p[0]),ny=std::min(p[1],y);
                w=std::max(x+w,p[0]+1.f)-nx;
                h=std::max(y+h,p[1]+1.f)-ny;x=nx;y=ny;
            }else{x=p[0];y=p[1];w=1.f;h=1.f;}
        }
        const float expansion=stroke_half+3.f;
        x=x-expansion;y=y-expansion;
        w=std::max((expansion+expansion)+w,0.f);
        h=std::max((expansion+expansion)+h,0.f);
        const auto left=static_cast<std::int32_t>(std::floor(x));
        const auto top=static_cast<std::int32_t>(std::floor(y));
        const auto right=static_cast<std::int32_t>(std::ceil(x+w));
        const auto bottom=static_cast<std::int32_t>(std::ceil(y+h));
        return std::array<std::int32_t,4>{left,top,right-left,bottom-top};
    };
    auto old=one(previous);const auto next=one(current);
    if(next[2]>0&&next[3]>0){
        if(old[2]<=0||old[3]<=0)return next;
        const auto x=std::min(old[0],next[0]),y=std::min(old[1],next[1]);
        const auto right=std::max(old[0]+old[2],next[0]+next[2]);
        const auto bottom=std::max(old[1]+old[3],next[1]+next[3]);
        old={x,y,right-x,bottom-y};
    }
    return old;
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

MagneticWheelResult magnetic_wheel(std::span<const MagneticZone> zones,float value,float vertical,bool wheel_reversed,bool inertial,bool reverse) noexcept {
 if(inertial||vertical==0.f)return {false,false,0.f};
 if(wheel_reversed)vertical=-vertical;
 const bool down=vertical<=0.f;if(reverse)value=1.f-value;
 const auto* before=zone_for_value(zones,value);if(!before||before->position_from==before->position_to)return {false,false,0.f};
 if(before->value_from!=before->value_to&&(before->value_from!=value||!down)){
  if(value!=before->value_to||down)return {false,false,0.f};
 }
 const MagneticZone* target=nullptr;float best=0.f;
 for(const auto& zone:zones){
  if(&zone==before)continue;
  if(down){if(zone.position_to>before->position_from)continue;const float distance=before->position_from-zone.position_to;if(!target||distance<best){target=&zone;best=distance;}}
  else {if(zone.position_from<before->position_to)continue;const float distance=zone.position_from-before->position_to;if(!target||distance<best){target=&zone;best=distance;}}
 }
 if(!target)return {true,false,0.f};
 const float next=down?target->value_to:target->value_from;
 if((down&&value<=next)||(!down&&next<=value))return {false,false,0.f};
 return {true,true,next};
}

void magnetic_phase(MagneticGesture& state,std::uint32_t phase,bool matches,float current,bool reverse) noexcept {
 if(!matches){state.held_value=-1.f;return;}
 if(reverse)current=1.f-current;
 if(phase==0){state.held_value=current;state.flag20=true;state.flag21=false;}
 else if(phase==2){const bool previous=state.flag20;state.flag20=previous&&current==state.held_value;state.flag21=previous&&!state.flag20;}
}

MagneticResult magnetic_delta(std::span<const MagneticZone> zones,
    float value,float delta,bool flag20,bool flag21,bool wrap,bool matches) noexcept {
 if(!matches)return {-1.f,false};
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
 position=position+delta;
 if(wrap){assert(!zones.empty()&&zones.back().position_to>0.f);const auto period=zones.back().position_to;position=std::fmod(position,period);if(position<0.f)position=position+period;}
 float result=position;
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
