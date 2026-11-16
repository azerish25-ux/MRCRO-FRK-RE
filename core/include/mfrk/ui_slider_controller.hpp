#pragma once
#include "mfrk/ui_slider_drag.hpp"
namespace mfrk::ui {
struct SliderPointerConfig {
 bool disabled_bit40,has_parent,enabled_override,suppress_drag,button_gate;
 std::uint32_t drag_phase;
};
struct SliderPointerState {bool dragged;std::uint32_t parameter_to_paint;};
// Host owns actual controller/metadata, coordinate transform, pointer capture
// and feature dispatch. info(...,true) must return the owner's custom info.
// blocked(0xffffffff) queries selected-parameter metadata, as in source drag.
struct SliderControllerHost:SliderBeginBoundary,SliderValueBoundary {
 virtual bool parent_enabled()=0;
 virtual std::int32_t pointer_mode()=0;
 virtual void enable_unbounded_pointer()=0;
 virtual bool phase(std::uint32_t,const SliderDragEvent&,std::uint32_t& id)=0;
 virtual float current(std::uint32_t id)=0;
 virtual MagneticResult map(float value,float delta,std::uint32_t id)=0;
 virtual void after_down(const SliderDragEvent&)=0;
 virtual void after_drag(const SliderDragEvent&)=0;
};
// ISlider source routing c450e0/c45370, c45db0/c45d10 plus recovered
// begin/drag/value bodies. Supplied state/config remain live across callbacks.
// The host must implement real GUI services; no plain-assignment substitute is
// installed here. finite component profiles, serial access, stable host lifetime.
class SliderController {
 SliderPointerConfig& pointer_config_;
 SliderPointerState& pointer_state_;
 SliderDragConfig& drag_config_;
 SliderDragState& drag_state_;
 SliderValueConfig& value_config_;
 SliderControllerHost& host_;
 bool allowed();
public:
 SliderController(SliderPointerConfig& p,SliderPointerState& s,SliderDragConfig& d,
                  SliderDragState& t,SliderValueConfig& v,SliderControllerHost& h)
  :pointer_config_(p),pointer_state_(s),drag_config_(d),drag_state_(t),value_config_(v),host_(h){}
 void down(const SliderDragEvent&);
 void drag(const SliderDragEvent&);
 float write(float view_value,std::uint32_t id,bool request_custom,bool precise);
 float write_selected(float view_value);
};
}
