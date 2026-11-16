#pragma once
#include "mfrk/ui.hpp"
namespace mfrk::ui {
// Source c45b50 metadata projection. Finite coefficients/override, dimensions
// >0, enum count 0..4096. Manager queries are supplied immutable metadata.
struct SliderSensitivity {
 float override_value,normal,fine;
 std::int32_t orientation,width,height,item_count;
 std::uint32_t modifiers;
 bool fine_enabled,enum_adjust,by_size,is_enum;
};
float slider_sensitivity(const SliderSensitivity&) noexcept;
struct SliderDragState {
 float value_anchor;std::uint32_t parameter_id;float sensitivity,global_anchor,local_anchor;
 bool global_tracking,outside;
};
struct SliderDragConfig {
 std::int32_t orientation;float fine_threshold;
 bool wrap,flag1640,custom_info;
};
struct SliderDragEvent {std::int32_t x,y;};
struct SliderDragBoundary {
 virtual ~SliderDragBoundary()=default;
 virtual std::array<std::int32_t,2> global_position(const SliderDragEvent&)=0;
 virtual MagneticResult map(float value,float delta,std::uint32_t id)=0;
 virtual std::uint32_t selected_id()=0;
 virtual float write(float value,std::uint32_t id,bool custom_info,bool precise)=0;
};
// Whole c454f0 tracking/control projection with supplied coordinate, feature-map
// and writer boundaries. Shared config/state can be updated by those boundaries.
// Native source coordinates are signed integers; orientation==1 is horizontal.
// Finite values/intermediates, source-initialized tracking, no exceptions and
// serial access required. Begin-edit/phase routing, sensitivity queries, actual
// coordinate transform, slider conversion/setter and physical touch are separate.
void slider_drag(SliderDragState&,SliderDragConfig&,const SliderDragEvent&,
 std::uint32_t id,float current,float sensitivity,bool apply,bool force_reset,
 SliderDragBoundary&);
}
