#pragma once
#include "mfrk/ui_pointer_geometry.hpp"
namespace mfrk::ui {
enum class GuiComponentMouseKind:std::uint32_t {move=0,enter=1,exit=2,down=3,drag=4,up=5,double_click=6};
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
template<class T>inline T gui_mouse_press_field(const std::array<std::uint8_t,32>& record,unsigned offset){T value;std::memcpy(&value,record.data()+offset,sizeof(T));return value;}
// Source ffb750 uses binary64 seconds for its timing comparison and compares
// the two packed modifier/peer words through their binary32 representations.
inline std::uint8_t gui_component_mouse_clicks(const GuiMouseSourceState& source){
 const auto& press=source.presses[0];const auto press_time=gui_mouse_press_field<std::int64_t>(press,8);
 if(source.dragged||press_time+300<source.last_time)return 1;
 unsigned clicks=1;
 for(unsigned i=1;i<4;++i){
  const auto& previous=source.presses[i];
  if(static_cast<double>(std::min(i,2u)*400)*.001<=static_cast<double>(press_time-gui_mouse_press_field<std::int64_t>(previous,8))*.001)return static_cast<std::uint8_t>(clicks);
  const float tolerance=press[24]!=0?25.f:8.f;
  if(tolerance<=std::abs(gui_mouse_press_field<float>(press,0)-gui_mouse_press_field<float>(previous,0))||tolerance<=std::abs(gui_mouse_press_field<float>(press,4)-gui_mouse_press_field<float>(previous,4)))return static_cast<std::uint8_t>(clicks);
  if(gui_mouse_press_field<float>(press,16)!=gui_mouse_press_field<float>(previous,16)||gui_mouse_press_field<float>(press,20)!=gui_mouse_press_field<float>(previous,20))return static_cast<std::uint8_t>(clicks);
  ++clicks;
 }
 return static_cast<std::uint8_t>(clicks);
}
struct GuiComponentMouseButtonBoundary:GuiComponentMouseBoundary {
 virtual void modal_input_attempt()=0;
 virtual void to_front(GuiVisibilityComponent&,bool take_focus)=0;
 virtual void grab_mouse_keyboard_focus(GuiVisibilityComponent&)=0;
};
inline GuiComponentMouseEvent gui_component_mouse_press_event(GuiVisibilityComponent& component,GuiMouseSourceState& source,GuiMousePoint point,std::int64_t time,std::uint32_t modifiers,const GuiMouseExtra& extra,GuiPointerGeometryBoundary& geometry){
 const auto& press=source.presses[0];
 const auto press_time=gui_mouse_press_field<std::int64_t>(press,8);
 const bool dragged=source.dragged||press_time+300<source.last_time;
 const auto clicks=gui_component_mouse_clicks(source);
 const GuiMousePoint screen{gui_mouse_press_field<float>(press,0),gui_mouse_press_field<float>(press,4)};
 const auto down=gui_pointer_map_point(&component,nullptr,gui_pointer_unscale(screen,geometry.desktop_scale()),geometry);
 return {point,{gui_pointer_round(point.x),gui_pointer_round(point.y)},modifiers,extra,down,&component,&component,time,press_time,&source,clicks,dragged};
}
// Source ff8330 and ff8050. Release keeps its supplied pre-release modifiers,
// and double-click follows all up listeners with a fresh weak-token check.
inline void gui_component_mouse_drag(GuiVisibilityComponent& component,GuiMouseSourceState& source,GuiMousePoint point,std::int64_t time,const GuiMouseExtra& extra,GuiPointerGeometryBoundary& geometry,GuiComponentMouseBoundary& boundary){
 if(boundary.modal_blocked(component))return;
 const auto guard=component.lifetime_guard();
 // Capture click/movement and coordinate services before current modifiers.
 auto event=gui_component_mouse_press_event(component,source,point,time,0,extra,geometry);
 event.modifiers=source.modifiers|(boundary.global_mouse_modifiers()&0xffffff8fu);
 boundary.component_mouse_event(component,GuiComponentMouseKind::drag,event);
 if(guard->component){gui_component_mouse_globals(guard,GuiComponentMouseKind::drag,event,boundary);gui_component_mouse_listeners(component,guard,GuiComponentMouseKind::drag,event);}
}
inline void gui_component_mouse_up(GuiVisibilityComponent& component,GuiMouseSourceState& source,GuiMousePoint point,std::int64_t time,std::uint32_t modifiers,const GuiMouseExtra& extra,GuiPointerGeometryBoundary& geometry,GuiComponentMouseBoundary& boundary){
 if((component.interaction_flags&1u)!=0&&boundary.modal_blocked(component))return;
 const auto guard=component.lifetime_guard();
 if((component.focus_flags&0x20u)!=0)boundary.repaint(component);
 const auto event=gui_component_mouse_press_event(component,source,point,time,modifiers,extra,geometry);
 boundary.component_mouse_event(component,GuiComponentMouseKind::up,event);
 if(guard->component){
  gui_component_mouse_globals(guard,GuiComponentMouseKind::up,event,boundary);gui_component_mouse_listeners(component,guard,GuiComponentMouseKind::up,event);
  if(guard->component&&event.clicks>1){
   boundary.component_mouse_event(component,GuiComponentMouseKind::double_click,event);
   if(guard->component){
    auto& listeners=boundary.global_mouse_listeners();auto index=static_cast<std::int32_t>(listeners.size());
    while(guard->component&&index>0){--index;if(listeners.size()<=static_cast<std::size_t>(index)){index=static_cast<std::int32_t>(listeners.size())-1;if(index<0)break;}listeners[static_cast<std::size_t>(index)]->mouse_event(GuiComponentMouseKind::double_click,event);}
    gui_component_mouse_listeners(component,guard,GuiComponentMouseKind::double_click,event);
   }
  }
 }
}
inline GuiComponentMouseEvent gui_component_mouse_down_event(GuiVisibilityComponent& component,GuiMouseSourceState& source,GuiMousePoint point,std::int64_t time,const GuiMouseExtra& extra,GuiComponentMouseBoundary& boundary){
 const auto clicks=gui_component_mouse_clicks(source);
 return {point,{gui_pointer_round(point.x),gui_pointer_round(point.y)},source.modifiers|(boundary.global_mouse_modifiers()&0xffffff8fu),extra,point,&component,&component,time,time,&source,clicks,false};
}
// Source ff7c80. Modal input attempts may unblock or delete the target. Normal
// down brings flagged ancestors forward, then applies source focus admission.
inline void gui_component_mouse_down(GuiVisibilityComponent& component,GuiMouseSourceState& source,GuiMousePoint point,std::int64_t time,const GuiMouseExtra& extra,GuiComponentMouseButtonBoundary& boundary){
 const auto guard=component.lifetime_guard();
 if(boundary.modal_blocked(component)){
  component.interaction_flags|=1u;boundary.modal_input_attempt();
  if(!guard->component)return;
  if(boundary.modal_blocked(component)){const auto event=gui_component_mouse_down_event(component,source,point,time,extra,boundary);gui_component_mouse_globals(guard,GuiComponentMouseKind::down,event,boundary);return;}
 }
 component.interaction_flags&=static_cast<std::uint8_t>(~1u);
 for(auto* node=&component;node;node=static_cast<GuiVisibilityComponent*>(node->parent))if((node->focus_flags&0x10u)!=0){boundary.to_front(*node,true);if(!guard->component)return;}
 if((component.focus_flags&2u)==0){boundary.grab_mouse_keyboard_focus(component);if(!guard->component)return;}
 if((component.focus_flags&0x20u)!=0)boundary.repaint(component);
 const auto event=gui_component_mouse_down_event(component,source,point,time,extra,boundary);
 boundary.component_mouse_event(component,GuiComponentMouseKind::down,event);
 if(guard->component){gui_component_mouse_globals(guard,GuiComponentMouseKind::down,event,boundary);gui_component_mouse_listeners(component,guard,GuiComponentMouseKind::down,event);}
}
// Actual pointer owner -> geometry -> component event/listener delivery join.
// Application adapters still provide platform services and real widget methods.
struct GuiMouseComponentBinding:GuiMouseGeometryBinding {
 GuiComponentMouseButtonBoundary& components;
 GuiMouseComponentBinding(GuiPointerGeometryBoundary& coordinates,GuiComponentMouseButtonBoundary& events):GuiMouseGeometryBinding(coordinates),components(events){}
 void mouse_enter(GuiVisibilityComponent& n,GuiMouseSourceState& s,GuiMousePoint p,std::int64_t t)final{gui_component_mouse_hover(n,s,p,t,GuiComponentMouseKind::enter,components);}
 void mouse_exit(GuiVisibilityComponent& n,GuiMouseSourceState& s,GuiMousePoint p,std::int64_t t)final{gui_component_mouse_hover(n,s,p,t,GuiComponentMouseKind::exit,components);}
 void mouse_move(GuiVisibilityComponent& n,GuiMouseSourceState& s,GuiMousePoint p,std::int64_t t)final{gui_component_mouse_hover(n,s,p,t,GuiComponentMouseKind::move,components);}
 void mouse_down(GuiVisibilityComponent& n,GuiMouseSourceState& s,GuiMousePoint p,std::int64_t t,const GuiMouseExtra& e)final{gui_component_mouse_down(n,s,p,t,e,components);}
 void mouse_drag(GuiVisibilityComponent& n,GuiMouseSourceState& s,GuiMousePoint p,std::int64_t t,const GuiMouseExtra& e)final{gui_component_mouse_drag(n,s,p,t,e,geometry,components);}
 void mouse_up(GuiVisibilityComponent& n,GuiMouseSourceState& s,GuiMousePoint p,std::int64_t t,std::uint32_t m,const GuiMouseExtra& e)final{gui_component_mouse_up(n,s,p,t,m,e,geometry,components);}
};
// Complete acyclic component ancestors, listener counts fitting signed32 and a
// valid deep prefix are required. Active registration objects survive dispatch.
// Callbacks can retire nodes, reparent living nodes and mutate listener arrays.
}
