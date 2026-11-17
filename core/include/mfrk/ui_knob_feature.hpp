#pragma once
#include "mfrk/ui.hpp"
#include "mfrk/ui_control_feature.hpp"
namespace mfrk::ui {
struct GuiKnobDisplayGeometry {
 std::int32_t width,height;
 float right_angle,left_angle,radius,thickness;
};
// Actual component services: the factory supplies its current style getters,
// repaint scheduler, scalar-range callback and component visibility owner.
// Style fields must stay stable while one range invalidation is calculated.
struct GuiKnobDisplayBoundary {
 virtual ~GuiKnobDisplayBoundary()=default;
 virtual GuiKnobDisplayGeometry geometry()=0;
 virtual void invalidate(const std::array<std::int32_t,4>& bounds)=0;
 virtual void invalidate()=0;
 virtual void scalar_changed(float previous,float current)=0;
 virtual void set_component_state(bool active)=0;
};
// Connects the actual registered controller rows and feature history to the
// source knob's f67ea0 range callback, then its component repaint scheduler.
// Scalar callbacks remain the separate f67c80 path owned by the scene.
class GuiKnobFeatureBinding final:public GuiControlFeatureBoundary {
 GuiControlParameters& parameters_;GuiKnobDisplayBoundary& component_;
public:
 GuiKnobFeatureBinding(GuiControlParameters& parameters,GuiKnobDisplayBoundary& component)
  :parameters_(parameters),component_(component){}
 std::uint32_t parameter_id(std::uint64_t index)override{return parameters_.id(index);}
 float current(std::uint32_t id)override{return parameters_.find(id).current();}
 bool is_data(std::uint32_t id)override{return parameters_.find(id).is_data();}
 std::span<const std::uint8_t> data(std::uint32_t id)override{return parameters_.find(id).current_data();}
 void scalar_changed(float previous,float current)override{component_.scalar_changed(previous,current);}
 void range_changed(const std::array<float,5>& previous,const std::array<float,5>& current)override{
  const auto g=component_.geometry();
  component_.invalidate(knob_modulation_dirty_bounds(g.width,g.height,g.right_angle,g.left_angle,g.radius,g.thickness,previous,current));
 }
 void invalidate()override{component_.invalidate();}
 void set_component_state(bool active)override{component_.set_component_state(active);}
};
}
