#pragma once
#include "mfrk/ui_pointer_geometry.hpp"
namespace mfrk::ui {
enum class GuiComponentMouseKind:std::uint32_t {move=0,enter=1,exit=2};
struct GuiComponentMouseEvent {
 GuiMousePoint position;
 std::array<std::int32_t,2> integer_position;
 std::uint32_t modifiers;
 GuiMouseExtra extra{};
 GuiMousePoint mouse_down_position;
 GuiVisibilityComponent* component;
 GuiVisibilityComponent* original_component;
 std::int64_t time,mouse_down_time;
 GuiMouseSourceState* source;
 std::uint8_t clicks=0;
 bool dragged=false;
};
struct GuiComponentMouseListener {
 virtual ~GuiComponentMouseListener()=default;
 virtual void mouse_event(GuiComponentMouseKind,const GuiComponentMouseEvent&)=0;
};
// Source component+90: deep listeners occupy the prefix, followed by local-only
// listeners. Registration owners retain the object while a traversal uses it.
struct GuiComponentMouseListeners {
 std::vector<GuiComponentMouseListener*> listeners;
 std::int32_t deep_count=0;
};
struct GuiComponentMouseBoundary {
 virtual ~GuiComponentMouseBoundary()=default;
 virtual bool modal_blocked(GuiVisibilityComponent&)=0;
 virtual void repaint(GuiVisibilityComponent&)=0;
 virtual std::uint32_t global_mouse_modifiers()=0;
 virtual void component_mouse_event(GuiVisibilityComponent&,GuiComponentMouseKind,const GuiComponentMouseEvent&)=0;
 virtual void desktop_mouse_prepare()=0;
 virtual void desktop_mouse_blocked_move()=0;
 virtual void hide_cursor(GuiMouseSourceState&)=0;
 virtual std::vector<GuiComponentMouseListener*>& global_mouse_listeners()=0;
};
// Source108e330. Local listeners run before deep listeners on each ancestor.
// Both weak tokens are checked after ancestor callbacks; a deleted/reparented
// ancestor is never subsequently dereferenced through its former pointer.
inline void gui_component_mouse_listeners(GuiVisibilityComponent& component,const std::shared_ptr<GuiComponentLifetime>& guard,GuiComponentMouseKind kind,const GuiComponentMouseEvent& event){
 if(!guard||!guard->component)return;
 if(auto* list=component.mouse_listeners){
  auto index=static_cast<std::int32_t>(list->listeners.size());
  while(--index>=0){
   list->listeners[static_cast<std::size_t>(index)]->mouse_event(kind,event);
   if(!guard->component)return;
   if(static_cast<std::int32_t>(list->listeners.size())<index)index=static_cast<std::int32_t>(list->listeners.size());
  }
 }
 for(auto* ancestor=static_cast<GuiVisibilityComponent*>(component.parent);ancestor;ancestor=static_cast<GuiVisibilityComponent*>(ancestor->parent)){
  if(auto* list=ancestor->mouse_listeners;list&&list->deep_count>0){
   const auto ancestor_guard=ancestor->lifetime_guard();auto index=list->deep_count;
   while(--index>=0){
    list->listeners[static_cast<std::size_t>(index)]->mouse_event(kind,event);
    if(!guard->component||!ancestor_guard->component)return;
    if(list->deep_count<index)index=list->deep_count;
   }
  }
 }
}
inline void gui_component_mouse_globals(const std::shared_ptr<GuiComponentLifetime>& guard,GuiComponentMouseKind kind,const GuiComponentMouseEvent& event,GuiComponentMouseBoundary& boundary){
 boundary.desktop_mouse_prepare();auto& listeners=boundary.global_mouse_listeners();
 auto index=static_cast<std::int32_t>(listeners.size());
 while(guard&&guard->component&&index>0){
  --index;
  if(listeners.size()<=static_cast<std::size_t>(index)){index=static_cast<std::int32_t>(listeners.size())-1;if(index<0)break;}
  listeners[static_cast<std::size_t>(index)]->mouse_event(kind,event);
 }
}
// Source ff78a0/ff7a90/ff8570: hover changes have different positions relative
// to the component callback. The event retains source and original component
// identity, and source mouse buttons are merged with current nonmouse modifiers.
inline void gui_component_mouse_hover(GuiVisibilityComponent& component,GuiMouseSourceState& source,GuiMousePoint point,std::int64_t time,GuiComponentMouseKind kind,GuiComponentMouseBoundary& boundary){
 if(boundary.modal_blocked(component)){
  if(kind==GuiComponentMouseKind::move)boundary.desktop_mouse_blocked_move();else boundary.hide_cursor(source);
  return;
 }
 if(kind!=GuiComponentMouseKind::move&&(component.focus_flags&0x20u)!=0)boundary.repaint(component);
 if(kind==GuiComponentMouseKind::exit)component.interaction_flags&=static_cast<std::uint8_t>(~0x20u);
 const auto guard=component.lifetime_guard();
 const GuiComponentMouseEvent event{point,{gui_pointer_round(point.x),gui_pointer_round(point.y)},source.modifiers|(boundary.global_mouse_modifiers()&0xffffff8fu),{},point,&component,&component,time,time,&source,0,false};
 boundary.component_mouse_event(component,kind,event);
 // Source enter writes this flag even when the callback expires its weak token;
 // callers retain component storage through the direct callback/flag write.
 if(kind==GuiComponentMouseKind::enter)component.interaction_flags|=0x20u;
 if(guard&&guard->component){gui_component_mouse_globals(guard,kind,event,boundary);gui_component_mouse_listeners(component,guard,kind,event);}
}
// Complete acyclic component ancestors, listener counts fitting signed32 and a
// valid deep prefix are required. Active registration objects survive dispatch.
// Callbacks can retire nodes, reparent living nodes and mutate listener arrays.
}
