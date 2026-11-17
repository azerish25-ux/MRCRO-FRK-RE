#pragma once
#include "mfrk/ui_component_repaint.hpp"
#include "mfrk/ui_component_visibility.hpp"
#include "mfrk/ui_knob_feature.hpp"
namespace mfrk::ui {
// Scene-owned display boundary: generated knob range/scalar dirty regions enter
// the actual component tree's clipping/cache/transforms/peer repaint path.
// The scene supplies live style, cached images, focus/mouse/peer and listener
// services. The component and presentation retain their shared flag byte.
class GuiKnobComponentDisplayBoundary:public GuiKnobDisplayBoundary {
 GuiVisibilityComponent& component_;GuiComponentVisibilityBoundary& repaint_;
 GuiSliderPresentation& presentation_;
public:
 GuiKnobComponentDisplayBoundary(GuiVisibilityComponent& component,GuiComponentVisibilityBoundary& repaint,GuiSliderPresentation& presentation)
  :component_(component),repaint_(repaint),presentation_(presentation){
  if(presentation_.component_flags_source)throw std::logic_error("Presentation component flags already bound");
  presentation_.component_flags_source=&component_.flags;
 }
 GuiKnobComponentDisplayBoundary(const GuiKnobComponentDisplayBoundary&)=delete;
 GuiKnobComponentDisplayBoundary& operator=(const GuiKnobComponentDisplayBoundary&)=delete;
 ~GuiKnobComponentDisplayBoundary()override{
  if(presentation_.component_flags_source==&component_.flags){presentation_.component_flags=component_.flags;presentation_.component_flags_source=nullptr;}
 }
 void invalidate(const GuiDirtyBounds& bounds)final{
  auto clipped=bounds;gui_component_repaint(component_,clipped,repaint_);
 }
 void invalidate()final{gui_component_full_repaint(component_,repaint_);}
 void set_component_state(bool visible)final{gui_widget_set_visible(component_,visible,repaint_);}
};
}
