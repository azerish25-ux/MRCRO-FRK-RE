#pragma once
#include "mfrk/ui_component_focus.hpp"
namespace mfrk::ui {
struct GuiGlobalFocusListener {
 virtual ~GuiGlobalFocusListener()=default;
 virtual void global_focus_changed(GuiVisibilityComponent*)=0;
};
// Source Desktop::handleAsyncUpdate ffd3d0. The focused component is captured
// once as a non-retaining weak token. Every listener reads that token's current
// pointer; a redirected global focus does not replace this dispatch's snapshot.
class GuiDesktopFocusUpdates final:public GuiAsyncUpdateTarget {
 GuiComponentFocusState& state_;
 std::vector<GuiGlobalFocusListener*>& listeners_;
 GuiComponentFocusUpdate update_;
public:
 GuiDesktopFocusUpdates(GuiComponentFocusState& state,std::vector<GuiGlobalFocusListener*>& listeners,GuiAsyncUpdateQueue& queue)
  :state_(state),listeners_(listeners),update_(state,*this,queue){}
 GuiDesktopFocusUpdates(const GuiDesktopFocusUpdates&)=delete;
 GuiDesktopFocusUpdates& operator=(const GuiDesktopFocusUpdates&)=delete;
 bool post_message(){return update_.post_message();}
 void trigger(){update_.trigger();}
 void cancel()noexcept{update_.cancel();}
 void handle_async_update()override{
  const auto snapshot=state_.focused?state_.focused->lifetime_guard():std::shared_ptr<GuiComponentLifetime>{};
  auto index=static_cast<std::int32_t>(listeners_.size());
  while(index>0){
   --index;
   if(listeners_.size()<=static_cast<std::size_t>(index)){
    index=static_cast<std::int32_t>(listeners_.size())-1;if(index<0)break;
   }
   listeners_[static_cast<std::size_t>(index)]->global_focus_changed(snapshot?snapshot->component:nullptr);
  }
 }
 // The Desktop/registry owner survives notification dispatch. Listeners and
 // focused nodes may change during callbacks; live list sizes fit signed32.
};
// Concrete visibility -> focus owner -> retained message -> Desktop listeners.
// Derived application boundaries still supply real peer/input/accessibility
// operations and the registered listener objects; no successful defaults.
struct GuiOwnedComponentFocusBoundary:GuiComponentFocusBoundary {
 GuiDesktopFocusUpdates focus_updates;
 GuiOwnedComponentFocusBoundary(GuiComponentFocusState& state,std::vector<GuiGlobalFocusListener*>& listeners,GuiAsyncUpdateQueue& queue)
  :GuiComponentFocusBoundary(state),focus_updates(state,listeners,queue){}
 bool post_focus_update()final{return focus_updates.post_message();}
};
}
