#include "mfrk/ui_slider_drag.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
namespace mfrk::ui {
float slider_start_offset(SliderValueInfo& info,float view_value){
 const auto mapped=slider_to_mapped(info,view_value);
 const auto normal=slider_format_roundtrip(mapped,info.precision);
 const auto fine=slider_format_roundtrip(mapped,info.fine_precision);
 return fine-normal;
}
void slider_begin(SliderDragState& state,SliderDragConfig& config,SliderValueConfig& value_config,
 const SliderDragEvent& event,std::uint32_t modifiers,std::uint32_t id,float view_value,
 SliderBeginBoundary& boundary){
 boundary.prepare(id);
 auto& info=boundary.info(id,config.custom_info);
 value_config.offset=slider_start_offset(info,view_value);
 boundary.edit(id,(modifiers>>5)&0xffffff01u,false);
 if(boundary.blocked(id))return;
 state.value_anchor=view_value;state.parameter_id=id;
 state.sensitivity=boundary.sensitivity(event,id);
 const bool horizontal=config.orientation==1;
 const auto point=boundary.global_position(event);
 state.global_anchor=static_cast<float>(point[horizontal?0:1]);
 state.global_tracking=false;
 state.local_anchor=static_cast<float>(config.orientation==1?event.x:event.y);
}
float slider_sensitivity(const SliderSensitivity& p) noexcept {
 if(std::abs(p.override_value)!=1000.f)return p.override_value;
 float result;
 if(p.by_size){const auto extent=p.orientation==1?p.width:p.height;assert(extent>0);result=1.f/static_cast<float>(extent);}
 else {
  const bool fine=p.fine_enabled&&(p.modifiers&0x22)!=0;
  result=fine?p.fine:p.normal;
  if(p.is_enum&&p.enum_adjust){const float steps=static_cast<float>(p.item_count-1);
   if(steps>0.f){
    if(fine)result=std::max(result,.005f/steps);
    else if(steps<=25.f)result=std::max(result,.02f/steps);
    else result=std::bit_cast<float>(0x3d888889u)/steps;
   }
  }
 }
 return p.override_value<0.f?-result:result;
}
void slider_drag(SliderDragState& state,SliderDragConfig& config,const SliderDragEvent& event,
 std::uint32_t id,float current,float sensitivity,bool apply,bool force_reset,SliderDragBoundary& boundary){
 auto global_axis=[&](){const bool horizontal=config.orientation==1;const auto point=boundary.global_position(event);return static_cast<float>(point[horizontal?0:1]);};
 auto local_axis=[&](){return static_cast<float>(config.orientation==1?event.x:event.y);};
 if(state.parameter_id==id&&!force_reset){if(sensitivity!=state.sensitivity)state.global_tracking=true;}
 else {state.parameter_id=id;state.value_anchor=current;state.global_anchor=global_axis();state.local_anchor=local_axis();state.sensitivity=sensitivity;}
 float distance;
 if(!state.global_tracking)distance=config.orientation==1?static_cast<float>(event.x)-state.local_anchor:state.local_anchor-static_cast<float>(event.y);
 else if(config.orientation==1){const auto point=boundary.global_position(event);distance=static_cast<float>(point[0])-state.global_anchor;}
 else {const auto point=boundary.global_position(event);distance=state.global_anchor-static_cast<float>(point[1]);}
 const float before=state.global_tracking?current:state.value_anchor;
 auto mapped=boundary.map(before,distance*sensitivity,id);
 if(config.wrap&&id==boundary.selected_id()){mapped.value=std::fmod(mapped.value,1.f);if(mapped.value<0.f)mapped.value=mapped.value+1.f;}
 state.outside=mapped.value>1.f||mapped.value<0.f;
 const float clamped=mapped.value<0.f?0.f:mapped.value<=1.f?mapped.value:1.f;
 bool refresh=mapped.skip;
 if(config.flag1640&&((distance<0.f&&clamped==0.f)||(distance>0.f&&clamped==1.f)))refresh=true;
 float actual=clamped;
 if(apply&&clamped!=current)actual=boundary.write(clamped,id,config.custom_info,config.fine_threshold<=sensitivity);
 if(refresh&&(actual!=before||clamped==before)){state.value_anchor=actual;state.local_anchor=local_axis();}
 if(actual!=before||refresh)state.global_anchor=global_axis();
}
}
