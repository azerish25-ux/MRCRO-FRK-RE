#pragma once
#include "mfrk/ui_component_visibility.hpp"
#include <atomic>
namespace mfrk::ui {
struct GuiComponentFocusState {
 GuiVisibilityComponent* focused=nullptr;
 std::uintptr_t accessibility_focused=0;
 std::atomic<std::uint32_t> update_pending{0};
};
struct GuiFocusPeer:GuiVisibilityPeer {
 virtual void grab_focus()=0;
 virtual bool focused()=0;
};
struct GuiKeyboardFocusTraverser {
 virtual ~GuiKeyboardFocusTraverser()=default;
 virtual GuiVisibilityComponent* default_component(GuiVisibilityComponent&)=0;
};
struct GuiComponentFocusBoundary;
inline void gui_component_grab_focus(GuiVisibilityComponent&,std::uint32_t,bool,GuiComponentFocusState&,GuiComponentFocusBoundary&);
inline void gui_component_give_away_focus(GuiVisibilityComponent&,bool,GuiComponentFocusState&,GuiComponentFocusBoundary&);
// Concrete ownership for the visibility boundary's focus operations. Every
// participating ancestor is an actual GuiVisibilityComponent scene node.
// Peer/traverser/accessibility/message callbacks are actual platform services.
struct GuiComponentFocusBoundary:GuiComponentVisibilityBoundary {
 GuiComponentFocusState& focus_state;
 explicit GuiComponentFocusBoundary(GuiComponentFocusState& state):focus_state(state){}
 virtual GuiFocusPeer* peer(GuiRepaintComponent&)override=0;
 virtual std::unique_ptr<GuiKeyboardFocusTraverser> keyboard_focus_traverser(GuiVisibilityComponent&)=0;
 virtual void focus_gained(GuiVisibilityComponent&,std::uint32_t reason)=0;
 virtual void focus_lost(GuiVisibilityComponent&,std::uint32_t reason)=0;
 virtual void child_focus_changed(GuiVisibilityComponent&,std::uint32_t reason)=0;
 virtual bool accessibility_is_ancestor(std::uintptr_t handler,std::uintptr_t focused)=0;
 virtual void request_accessibility_focus(std::uintptr_t handler,bool try_parent)=0;
 // Source f7b9b0 post result. Delivery/cancellation belongs to the actual
 // Desktop AsyncUpdater owner, which clears update_pending when consumed.
 virtual bool post_focus_update()=0;
 GuiRepaintComponent* focused_component()final{return focus_state.focused;}
 void grab_keyboard_focus(GuiRepaintComponent& component,std::uint32_t reason,bool try_parent)final{
  gui_component_grab_focus(static_cast<GuiVisibilityComponent&>(component),reason,try_parent,focus_state,*this);
 }
 void give_away_keyboard_focus(GuiVisibilityComponent& component,bool notify)final{
  gui_component_give_away_focus(component,notify,focus_state,*this);
 }
};
inline GuiVisibilityComponent* gui_visibility_parent(GuiVisibilityComponent& component){
 return static_cast<GuiVisibilityComponent*>(component.parent);
}
inline bool gui_focus_contains(const GuiVisibilityComponent& component,const GuiRepaintComponent* candidate,bool include_self){
 auto* node=candidate;if(node&&!include_self)node=node->parent;
 for(;node;node=node->parent)if(node==&component)return true;
 return false;
}
// Source ff9780; disabled state is checked along the actual ancestor chain.
inline bool gui_component_is_enabled(GuiVisibilityComponent& component){
 for(auto* node=&component;node;node=gui_visibility_parent(*node))if((node->focus_flags&0x40u)!=0)return false;
 return true;
}
// Source ff2ed0 + takeKeyboardFocus's self/parent admission.
inline GuiFocusPeer* gui_component_focus_peer(GuiVisibilityComponent& component,GuiComponentFocusBoundary& boundary){
 auto* node=&component;
 for(;;){if((node->flags&1u)!=0)return boundary.peer(*node);node=gui_visibility_parent(*node);if(!node)return nullptr;}
}
inline void gui_trigger_focus_update(GuiComponentFocusState& state,GuiComponentFocusBoundary& boundary){
 std::uint32_t expected=0;
 if(state.update_pending.compare_exchange_strong(expected,1))if(!boundary.post_focus_update())state.update_pending.exchange(0);
}
// Source ff8f30. The supplied token is checked after a changed-focus callback;
// unchanged flags still propagate to the current parent, as in the source.
inline void gui_component_child_focus_changed(GuiVisibilityComponent& component,std::uint32_t reason,
 const std::shared_ptr<GuiComponentLifetime>& guard,GuiComponentFocusState& state,GuiComponentFocusBoundary& boundary){
 const bool within=gui_focus_contains(component,state.focused,true);
 if(((component.focus_flags&1u)!=0)!=within){
  component.focus_flags=static_cast<std::uint8_t>((component.focus_flags&0xfeu)|(within?1u:0u));
  boundary.child_focus_changed(component,reason);
  if(!guard||!guard->component)return;
 }
 if(auto* parent=gui_visibility_parent(component)){
  const auto parent_guard=parent->lifetime_guard();
  gui_component_child_focus_changed(*parent,reason,parent_guard,state,boundary);
 }
}
// Source ff8e60. Keyboard focus loss precedes accessibility and ancestor state.
inline void gui_component_focus_lost(GuiVisibilityComponent& component,std::uint32_t reason,
 GuiComponentFocusState& state,GuiComponentFocusBoundary& boundary){
 const auto guard=component.lifetime_guard();boundary.focus_lost(component,reason);
 if(!guard->component)return;
 const auto handler=boundary.accessibility_handler(component);
 if(handler&&state.accessibility_focused){
  if(state.accessibility_focused==handler||boundary.accessibility_is_ancestor(handler,state.accessibility_focused)){
   state.accessibility_focused=0;boundary.notify_accessibility(handler,GuiAccessibilityEvent::focus_changed);
  }
 }
 gui_component_child_focus_changed(component,reason,guard,state,boundary);
}
// Source ff90d0. Guards borrow both components without extending their lives.
// The focused pointer changes before notifying the old component. Gain callbacks
// and ancestor state observe any further focus changes made by those callbacks.
inline void gui_component_take_focus(GuiVisibilityComponent& component,std::uint32_t reason,
 GuiComponentFocusState& state,GuiComponentFocusBoundary& boundary){
 if(state.focused==&component)return;
 auto* peer=gui_component_focus_peer(component,boundary);if(!peer)return;
 const auto guard=component.lifetime_guard();peer->grab_focus();
 if(!peer->focused()||state.focused==&component)return;
 auto* losing=state.focused;
 const auto losing_guard=losing?losing->lifetime_guard():std::shared_ptr<GuiComponentLifetime>{};
 state.focused=&component;gui_trigger_focus_update(state,boundary);
 if(losing_guard&&losing_guard->component)gui_component_focus_lost(*losing,reason,state,boundary);
 if(state.focused==&component){
  boundary.focus_gained(component,reason);
  if(guard->component){
   const auto handler=boundary.accessibility_handler(component);
   if(handler&&state.accessibility_focused!=handler)boundary.request_accessibility_focus(handler,true);
   gui_component_child_focus_changed(component,reason,guard,state,boundary);
  }
 }
}
// Source ff92b0. Traversers are actual owning results, destroyed after their
// recursive default-component attempt and before parent fallback. Traversal
// defaults/parent relationships must be acyclic, as in the valid source scene.
inline void gui_component_grab_focus(GuiVisibilityComponent& component,std::uint32_t reason,bool try_parent,
 GuiComponentFocusState& state,GuiComponentFocusBoundary& boundary){
 auto* node=&component;
 for(;;){
  if(!gui_component_is_showing(*node,boundary))return;
  if((node->flags&0x20u)!=0&&(gui_component_is_enabled(*node)||!node->parent)){
   gui_component_take_focus(*node,reason,state,boundary);return;
  }
  if(state.focused&&gui_focus_contains(*node,state.focused,false)&&gui_component_is_showing(*state.focused,boundary))return;
  {
   auto traverser=boundary.keyboard_focus_traverser(*node);
   if(traverser)if(auto* next=traverser->default_component(*node)){
    gui_component_grab_focus(*next,reason,false,state,boundary);return;
   }
  }
  if(!try_parent||!node->parent)return;
  node=gui_visibility_parent(*node);try_parent=true;
 }
}
// Source ff9450. Focus-loss callbacks may choose a new focused component;
// the subsequent coalesced Desktop update retains that callback ordering.
inline void gui_component_give_away_focus(GuiVisibilityComponent& component,bool notify,
 GuiComponentFocusState& state,GuiComponentFocusBoundary& boundary){
 if(!gui_focus_contains(component,state.focused,true))return;
 auto* losing=state.focused;
 if(losing){state.focused=nullptr;if(notify)gui_component_focus_lost(*losing,2,state,boundary);gui_trigger_focus_update(state,boundary);}
}
}
