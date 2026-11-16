#include "mfrk/ui_slider_controller.hpp"
namespace mfrk::ui {
bool SliderController::allowed(){
 if(!pointer_config_.disabled_bit40&&(!pointer_config_.has_parent||host_.parent_enabled()))return true;
 return pointer_config_.enabled_override;
}
float SliderController::write(float value,std::uint32_t id,bool request_custom,bool precise){
 auto& info=host_.info(id,request_custom&&drag_config_.custom_info);
 return slider_write(value_config_,info,id,value,precise,host_);
}
float SliderController::write_selected(float value){
 const auto id=host_.selected_id();
 auto& info=host_.info(id,drag_config_.custom_info);
 return slider_write(value_config_,info,id,value,false,host_);
}
void SliderController::down(const SliderDragEvent& event){
 if(!allowed())return;
 if(drag_config_.flag1640&&host_.pointer_mode()==0)host_.enable_unbounded_pointer();
 auto id=host_.selected_id();
 if(!host_.phase(0,event,id)){
  pointer_state_.parameter_to_paint=id;
  float value=host_.current(id);if(value_config_.reverse)value=1.f-value;
  slider_begin(drag_state_,drag_config_,value_config_,event,event.modifiers,id,value,host_);
 }
 host_.after_down(event);
}
void SliderController::drag(const SliderDragEvent& event){
 if(!allowed())return;
 pointer_state_.dragged=true;
 if(!pointer_config_.suppress_drag&&!host_.blocked(0xffffffffu)&&(!pointer_config_.button_gate||(event.modifiers&1)==0)){
  auto id=host_.selected_id();
  if(!host_.phase(pointer_config_.drag_phase,event,id)){
   pointer_state_.parameter_to_paint=id;
   const auto sensitivity=host_.sensitivity(event,id);
   float value=host_.current(id);if(value_config_.reverse)value=1.f-value;
   struct Boundary final:SliderDragBoundary {
    SliderController& owner;SliderControllerHost& host;
    Boundary(SliderController& o,SliderControllerHost& h):owner(o),host(h){}
    std::array<std::int32_t,2> global_position(const SliderDragEvent& e)override{return host.global_position(e);}
    MagneticResult map(float v,float d,std::uint32_t id)override{return host.map(v,d,id);}
    std::uint32_t selected_id()override{return host.selected_id();}
    float write(float v,std::uint32_t id,bool custom,bool precise)override{return owner.write(v,id,custom,precise);}
   } boundary(*this,host_);
   slider_drag(drag_state_,drag_config_,event,id,value,sensitivity,true,false,boundary);
  }
 }
 host_.after_drag(event);
}
}
