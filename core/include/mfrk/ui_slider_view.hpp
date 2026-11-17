#pragma once
#include "mfrk/ui_control_feature.hpp"
namespace mfrk::ui {
struct GuiSliderBounds {std::int32_t x,y,width,height;bool operator==(const GuiSliderBounds&)const=default;};
struct GuiSliderParent {std::uintptr_t identity;std::int32_t width,height;};
struct GuiSliderViewState {
 GuiSliderPresentation presentation;
 GuiSliderBounds bounds{};
 GuiSliderParent* parent=nullptr;
 std::uint64_t selected_index=0;
 std::int32_t orientation=0;
 bool movable=false;
 std::uintptr_t signal=0;
};
struct GuiSliderViewBoundary {
 virtual ~GuiSliderViewBoundary()=default;
 virtual std::uint32_t parameter_id(std::uint64_t index)=0;
 virtual float current(std::uint64_t index)=0;
 virtual void set_bounds(const GuiSliderBounds&)=0;
 virtual bool smart_parent(std::uintptr_t identity)=0;
 virtual void record_parent_bounds(std::uintptr_t identity,const GuiSliderBounds&)=0;
 virtual void invalidate()=0;
 virtual void signal(std::uintptr_t identity,double value)=0;
 virtual void feature_changed(std::uint32_t id)=0;
};
inline std::int32_t gui_view_truncate(float value)noexcept{
 if(!std::isfinite(value)||value<-2147483648.f||value>=2147483648.f)return std::numeric_limits<std::int32_t>::min();
 return static_cast<std::int32_t>(value);
}
// Sourcec44ea0. Live actual rows, bounds, parent and signals are supplied by the
// scene. Finite values and bounded dimensions/intermediates, stable parent
// lifetime, serial callbacks and no signed dimension subtraction overflow.
inline void gui_slider_view_changed(GuiSliderViewState& state,GuiSliderViewBoundary& boundary,std::uint32_t event){
 if(event!=boundary.parameter_id(state.selected_index))return;
 float value=boundary.current(state.selected_index);const bool reverse=state.presentation.reverse;
 if(reverse)value=1.f-value;
 if(parameter_value_bits::equal_scalar(std::bit_cast<std::uint32_t>(value),std::bit_cast<std::uint32_t>(state.presentation.view_cache)))return;
 if(state.movable&&state.parent){
  auto* parent=state.parent;const auto width=state.bounds.width,height=state.bounds.height;
  if(state.orientation==0){
   float position=value;if(!reverse)position=1.f-value;
   const auto y=gui_view_truncate(static_cast<float>(parent->height-height)*position);
   if(y!=state.bounds.y){
    boundary.set_bounds({state.bounds.x,y,width,height});
    if(boundary.smart_parent(parent->identity))boundary.record_parent_bounds(parent->identity,{state.bounds.x,y,width,height});
   }
  }else{
   float position=value;if(reverse)position=1.f-value;
   const auto x=gui_view_truncate(static_cast<float>(parent->width-width)*position);
   if(x!=state.bounds.x){
    boundary.set_bounds({x,state.bounds.y,width,height});
    if(boundary.smart_parent(parent->identity))boundary.record_parent_bounds(parent->identity,{x,state.bounds.y,width,height});
   }
  }
 }
 state.presentation.view_cache=value;boundary.invalidate();
 if(state.signal)boundary.signal(state.signal,static_cast<double>(value));
}
// Actuald4c300/c450d0 observer. The modulation/display feature runs even when
// the primary selected parameter did not change. Scalar origin6 writes only
// its separate cache, without an invented invalidation or value setter.
class GuiSliderViewObserver final:public GuiParameterObserver {
 GuiSliderViewState& state_;GuiSliderViewBoundary& boundary_;
public:
 GuiSliderViewObserver(GuiSliderViewState& state,GuiSliderViewBoundary& boundary):state_(state),boundary_(boundary){}
 void changed(std::uint32_t id)override{gui_slider_view_changed(state_,boundary_,id);boundary_.feature_changed(id);}
 void scalar_changed(std::uint32_t,float value)override{state_.presentation.scalar_cache=value;}
};
}
