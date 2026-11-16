#pragma once
#include "mfrk/ui.hpp"
#include "mfrk/ui_slider_value.hpp"
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
struct SliderDragEvent {std::int32_t x,y;std::uint32_t modifiers=0;};
struct SliderBeginBoundary {
 virtual ~SliderBeginBoundary()=default;
 virtual void prepare(std::uint32_t id)=0;
 virtual SliderValueInfo& info(std::uint32_t id,bool custom)=0;
 virtual void edit(std::uint32_t id,std::uint32_t modifiers,bool secondary)=0;
 virtual bool blocked(std::uint32_t id)=0;
 virtual float sensitivity(const SliderDragEvent&,std::uint32_t id)=0;
 virtual std::array<std::int32_t,2> global_position(const SliderDragEvent&)=0;
};
// Source c42dd0: fine formatted value minus normal formatted value, in mapped
// units. Both text roundtrips execute even if the precisions are equal.
float slider_start_offset(SliderValueInfo&,float view_value);
// Source c451d0 after the outer pointer/phase gate. The supplied callbacks retain
// controller/metadata ownership. Offset/edit execute even when tracking is
// blocked. Config and state may be changed by callbacks; calls are serial.
void slider_begin(SliderDragState&,SliderDragConfig&,SliderValueConfig&,
 const SliderDragEvent&,std::uint32_t modifiers,std::uint32_t id,float view_value,
 SliderBeginBoundary&);
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
