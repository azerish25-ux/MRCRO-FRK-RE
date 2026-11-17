#pragma once
#include "mfrk/ui_component_repaint.hpp"
#include "mfrk/ui_knob_feature.hpp"
namespace mfrk::ui {
// Scene-owned display boundary: generated knob range/scalar dirty regions enter
// the actual component tree's clipping/gates/transforms/peer repaint path.
// The scene still supplies live style getters and component
// state services. Their source bodies are not replaced by no-op defaults.
class GuiKnobComponentDisplayBoundary:public GuiKnobDisplayBoundary {
 GuiRepaintComponent& component_;GuiRepaintBoundary& repaint_;
public:
 GuiKnobComponentDisplayBoundary(GuiRepaintComponent& component,GuiRepaintBoundary& repaint)
  :component_(component),repaint_(repaint){}
 void invalidate(const GuiDirtyBounds& bounds)final{
  auto clipped=bounds;gui_component_repaint(component_,clipped,repaint_);
 }
 void invalidate()final{gui_component_full_repaint(component_,repaint_);}
};
}
