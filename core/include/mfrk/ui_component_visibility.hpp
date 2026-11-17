#pragma once
#include "mfrk/ui_component_repaint.hpp"
#include <map>
#include <memory>
#include <vector>
namespace mfrk::ui {
struct GuiVisibilityComponent;
struct GuiComponentLifetime {GuiVisibilityComponent* component;};
struct GuiComponentVisibilityListener {
 virtual ~GuiComponentVisibilityListener()=default;
 virtual void visibility_changed(GuiVisibilityComponent&)=0;
 virtual void hierarchy_changed(GuiVisibilityComponent&)=0;
};
struct GuiVisibilityHook {
 virtual ~GuiVisibilityHook()=default;
 virtual bool allow_visibility(GuiVisibilityComponent&,bool)=0;
};
struct GuiWidgetVisibilityListener {
 virtual ~GuiWidgetVisibilityListener()=default;
 virtual void changed()=0;
};
// The repaint byte/geometry are the same component fields used by visibility.
// Actual parent nodes and child pointers retain their scene-owned lifetimes.
struct GuiVisibilityComponent:GuiRepaintComponent {
 bool requested_visible=false;
 std::vector<GuiVisibilityComponent*> children;
 std::vector<GuiComponentVisibilityListener*> component_listeners;
 std::map<std::uintptr_t,GuiVisibilityHook*> visibility_hooks;
 std::vector<GuiWidgetVisibilityListener*> widget_listeners;
 std::uintptr_t requested_signal=0,show_signal=0,hide_signal=0,shown_signal=0,hidden_signal=0;
private:
 std::shared_ptr<GuiComponentLifetime> lifetime_;
public:
 GuiVisibilityComponent():GuiRepaintComponent{0,{},nullptr,nullptr,nullptr}{}
 GuiVisibilityComponent(const GuiVisibilityComponent&)=delete;
 GuiVisibilityComponent& operator=(const GuiVisibilityComponent&)=delete;
 ~GuiVisibilityComponent(){retire();}
 std::shared_ptr<GuiComponentLifetime> lifetime_guard(){
  if(!lifetime_)lifetime_=std::make_shared<GuiComponentLifetime>(GuiComponentLifetime{this});
  return lifetime_;
 }
 // Source WeakReference::Master clear: guards retain the token, never the node.
 void retire()noexcept{if(lifetime_)lifetime_->component=nullptr;}
};
struct GuiVisibilityPeer:GuiRepaintPeer {
 virtual bool minimized()=0;
 virtual void set_visible(bool)=0;
};
struct GuiComponentVisibilityBoundary:GuiRepaintBoundary {
 virtual GuiVisibilityPeer* peer(GuiRepaintComponent&)override=0;
 virtual void fake_mouse_move(GuiVisibilityComponent&)=0;
 virtual GuiRepaintComponent* focused_component()=0;
 virtual void grab_keyboard_focus(GuiRepaintComponent&,std::uint32_t reason,bool try_parent)=0;
 virtual void give_away_keyboard_focus(GuiVisibilityComponent&,bool notify)=0;
 virtual void visibility_changed(GuiVisibilityComponent&)=0;
 virtual void hierarchy_changed(GuiVisibilityComponent&)=0;
 virtual std::uintptr_t hierarchy_peer_handle(GuiVisibilityComponent&)=0;
 virtual void post_hierarchy_message(std::uintptr_t peer,std::uint32_t message)=0;
 virtual void effective_visibility_changed(GuiVisibilityComponent&,bool)=0;
 virtual void emit_visibility(std::uintptr_t signal,bool)=0;
 virtual void emit(std::uintptr_t signal)=0;
};
// Source ff2930. Peer lookup occurs only at a visible root with peer flag1.
inline bool gui_component_is_showing(GuiRepaintComponent& component,GuiComponentVisibilityBoundary& boundary){
 auto* node=&component;
 for(;;){if((node->flags&2u)==0)return false;if(!node->parent)break;node=node->parent;}
 if((node->flags&1u)==0)return false;
 auto* peer=boundary.peer(*node);return peer&&!peer->minimized();
}
// Source ff20e0. Cache release precedes capturing the child range. Child arrays
// remain stable while that captured range is traversed, as in the native code.
inline void gui_component_release_image_resources(GuiVisibilityComponent& component){
 if(auto* cache=component.cached_image)cache->release_resources();
 const auto count=component.children.size();
 for(std::size_t i=0;i<count;++i)gui_component_release_image_resources(*component.children[i]);
}
// Source ff5f50 for a normal child with stable parent during visible-bounds
// callbacks. Desktop-peer components have no parent in the admitted scene.
inline void gui_component_repaint_parent(GuiVisibilityComponent& component,GuiComponentVisibilityBoundary& boundary){
 if(auto* parent=component.parent){
  auto request=boundary.visible_bounds(component);
  request[0]-=component.bounds[0];request[1]-=component.bounds[1];
  request[0]+=component.bounds[0];request[1]+=component.bounds[1];
  if(component.transform)request=gui_dirty_transform(request,*component.transform);
  gui_component_repaint(*parent,request,boundary);
 }
}
// Source ff2880: reverse live listener array, rechecking both size and lifetime.
inline void gui_component_visibility_changed(GuiVisibilityComponent& component,GuiComponentVisibilityBoundary& boundary){
 const auto guard=component.lifetime_guard();boundary.visibility_changed(component);
 if(!guard->component)return;
 auto index=static_cast<std::int32_t>(component.component_listeners.size());
 while(guard->component&&index>0){
  --index;
  if(component.component_listeners.size()<=static_cast<std::size_t>(index)){
   index=static_cast<std::int32_t>(component.component_listeners.size())-1;if(index<0)break;
  }
  component.component_listeners[static_cast<std::size_t>(index)]->visibility_changed(component);
 }
}
// Source ff5740. Child traversal and listener traversal each recheck the live
// size after callbacks. The weak token permits deletion during notifications.
inline void gui_component_hierarchy_changed(GuiVisibilityComponent& component,GuiComponentVisibilityBoundary& boundary){
 const auto guard=component.lifetime_guard();boundary.hierarchy_changed(component);
 if(!guard->component)return;
 auto index=static_cast<std::int32_t>(component.component_listeners.size());
 while(guard->component&&index>0){
  --index;
  if(component.component_listeners.size()<=static_cast<std::size_t>(index)){
   index=static_cast<std::int32_t>(component.component_listeners.size())-1;if(index<0)break;
  }
  component.component_listeners[static_cast<std::size_t>(index)]->hierarchy_changed(component);
 }
 if(!guard->component)return;
 index=static_cast<std::int32_t>(component.children.size())-1;
 while(index>=0){
  gui_component_hierarchy_changed(*component.children[static_cast<std::size_t>(index)],boundary);
  if(!guard->component)return;
  if(component.children.size()<static_cast<std::size_t>(index))index=static_cast<std::int32_t>(component.children.size());
  --index;
 }
 if((component.flags&1u)!=0)if(const auto peer=boundary.hierarchy_peer_handle(component))boundary.post_hierarchy_message(peer,0x4e22u);
}
// Source ff2710. Serial acyclic live trees, bool requests, stable component
// storage through pre-notification services and signed32-sized arrays required.
// Platform mouse/focus/peer bodies are actual mandatory services, not defaults.
inline void gui_component_set_visible(GuiVisibilityComponent& component,bool visible,GuiComponentVisibilityBoundary& boundary){
 if(((component.flags&2u)!=0)==visible)return;
 const auto guard=component.lifetime_guard();
 component.flags=static_cast<std::uint8_t>((component.flags&0xfdu)|(visible?2u:0u));
 if(!visible){
  gui_component_repaint_parent(component,boundary);boundary.fake_mouse_move(component);
  gui_component_release_image_resources(component);
  auto* focus=boundary.focused_component();
  while(focus&&focus!=&component)focus=focus->parent;
  if(focus){
   if(component.parent)boundary.grab_keyboard_focus(*component.parent,2,true);
   boundary.give_away_keyboard_focus(component,true);
  }
 }else{gui_component_full_repaint(component,boundary);boundary.fake_mouse_move(component);}
 if(guard->component){
  gui_component_visibility_changed(component,boundary);
  if(guard->component&&(component.flags&1u)!=0)if(auto* peer=boundary.peer(component)){
   peer->set_visible(visible);gui_component_hierarchy_changed(component,boundary);
  }
 }
}
// Source b6eab0 Arturia widget wrapper. Every hook runs; denial skips the base
// transition/effective callback/listeners while retaining requested signals.
// Hook-map iterators and captured widget-listener ranges stay valid during
// their respective callbacks; the outer widget survives the complete call.
inline void gui_widget_set_visible(GuiVisibilityComponent& component,bool visible,GuiComponentVisibilityBoundary& boundary){
 const bool old_visible=(component.flags&2u)!=0;component.requested_visible=visible;
 bool admitted=true;
 for(auto it=component.visibility_hooks.begin();it!=component.visibility_hooks.end();++it)admitted&=it->second->allow_visibility(component,visible);
 if(admitted){
  gui_component_set_visible(component,visible,boundary);
  boundary.effective_visibility_changed(component,gui_component_is_showing(component,boundary));
  const auto count=component.widget_listeners.size();for(std::size_t i=0;i<count;++i)component.widget_listeners[i]->changed();
 }
 if(component.requested_signal)boundary.emit_visibility(component.requested_signal,visible);
 if(visible){
  if(component.show_signal)boundary.emit(component.show_signal);
  if(!old_visible&&component.shown_signal)boundary.emit(component.shown_signal);
 }else{
  if(component.hide_signal)boundary.emit(component.hide_signal);
  if(old_visible&&component.hidden_signal)boundary.emit(component.hidden_signal);
 }
}
}
