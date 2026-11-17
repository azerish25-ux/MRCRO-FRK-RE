#pragma once
#include "mfrk/ui_component_visibility.hpp"
#include "mfrk/ui_async_updater.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>
namespace mfrk::ui {
struct GuiMousePoint {float x,y;};
using GuiMouseExtra=std::array<std::uint32_t,5>;
struct GuiPointerPeer;
struct GuiMouseSourceState {
 GuiMousePoint position{},offset{};
 std::uint32_t modifiers=0,source_type=0,event_serial=0;
 GuiMouseExtra extra{};
 std::array<std::array<std::uint8_t,32>,4> presses{};
 bool unbounded=false,keep_cursor_visible=false,dragged=false;
 std::shared_ptr<GuiComponentLifetime> target;
 std::shared_ptr<GuiComponentLifetime> auxiliary_target; // Source weak slot+50.
 GuiPointerPeer* peer=nullptr; // Actual non-owning source peer, validated before hit lookup.
 std::int64_t last_time=0;
 std::atomic<std::uint32_t> pending{0};
 std::atomic<std::uint32_t>* pending_source=nullptr;
 std::atomic<std::uint32_t>& pending_word(){return pending_source?*pending_source:pending;}
 GuiVisibilityComponent* current_target()const noexcept{return target?target->component:nullptr;}
};
struct GuiMouseInputBoundary {
 virtual ~GuiMouseInputBoundary()=default;
 virtual GuiVisibilityComponent* find_target(GuiMouseSourceState&,GuiMousePoint screen)=0;
 virtual GuiMousePoint local_position(GuiVisibilityComponent&,GuiMousePoint screen)=0;
 virtual std::uint32_t global_modifiers()=0;
 virtual std::uint32_t peer_identifier(GuiVisibilityComponent&)=0;
 virtual void increment_mouse_down_count()=0;
 virtual void mouse_enter(GuiVisibilityComponent&,GuiMouseSourceState&,GuiMousePoint,std::int64_t)=0;
 virtual void mouse_exit(GuiVisibilityComponent&,GuiMouseSourceState&,GuiMousePoint,std::int64_t)=0;
 virtual void mouse_move(GuiVisibilityComponent&,GuiMouseSourceState&,GuiMousePoint,std::int64_t)=0;
 virtual void mouse_drag(GuiVisibilityComponent&,GuiMouseSourceState&,GuiMousePoint,std::int64_t,const GuiMouseExtra&)=0;
 virtual void mouse_down(GuiVisibilityComponent&,GuiMouseSourceState&,GuiMousePoint,std::int64_t,const GuiMouseExtra&)=0;
 virtual void mouse_up(GuiVisibilityComponent&,GuiMouseSourceState&,GuiMousePoint,std::int64_t,std::uint32_t,const GuiMouseExtra&)=0;
 virtual void update_cursor(GuiMouseSourceState&,bool force)=0;
 virtual GuiDirtyBounds global_bounds(GuiVisibilityComponent&)=0;
 virtual GuiDirtyBounds display_bounds(const GuiDirtyBounds&)=0;
 virtual float desktop_scale()=0;
 virtual GuiMousePoint scaled_to_unscaled(GuiMousePoint)=0;
 virtual GuiMousePoint unscaled_to_scaled(GuiMousePoint)=0;
 virtual void warp_cursor(GuiMousePoint)=0;
 virtual std::int64_t current_time_ms()=0;
};
// Source ffab60/ffa920/ffa580/ffb150/ffb280 state owner. Platform hit testing,
// transforms, event bodies, clock and cursor are actual mandatory services.
// Serial access, finite coordinates/intermediates and bounded integer geometry
// are required. Button state uses the source mouse-button mask. Component
// storage survives coordinate/cursor service calls.
class GuiMouseInputSource {
 GuiMouseSourceState& state_;GuiMouseInputBoundary& boundary_;
 static std::shared_ptr<GuiComponentLifetime> weak(GuiVisibilityComponent* node){return node?node->lifetime_guard():std::shared_ptr<GuiComponentLifetime>{};}
 static float press_float(const std::array<std::uint8_t,32>& record,unsigned offset){float value;std::memcpy(&value,record.data()+offset,4);return value;}
 template<class T>static void store(std::array<std::uint8_t,32>& record,unsigned offset,T value){std::memcpy(record.data()+offset,&value,sizeof(value));}
public:
 GuiMouseInputSource(GuiMouseSourceState& state,GuiMouseInputBoundary& boundary):state_(state),boundary_(boundary){}
 void unbounded(bool enable){
  const bool wanted=enable&&(state_.modifiers&0x70u)!=0;state_.keep_cursor_visible=false;
  if(wanted==state_.unbounded)return;
  if(!wanted)if(auto* target=state_.current_target()){
   const auto bounds=boundary_.global_bounds(*target);const auto current=boundary_.unscaled_to_scaled(state_.position);
   const float x=static_cast<float>(bounds[0]),y=static_cast<float>(bounds[1]);
   const float right=x+static_cast<float>(bounds[2]),bottom=y+static_cast<float>(bounds[3]);
   const GuiMousePoint clamped{current.x<x?x:current.x<=right?current.x:right,current.y<y?y:current.y<=bottom?current.y:bottom};
   boundary_.warp_cursor(boundary_.scaled_to_unscaled(clamped));
  }
  state_.unbounded=wanted;state_.offset={0.f,0.f};boundary_.update_cursor(state_,true);
 }
 void update_unbounded(GuiVisibilityComponent& target){
  const auto display=boundary_.display_bounds(boundary_.global_bounds(target));
  const auto width=std::max(0,display[2]-4),height=std::max(0,display[3]-4);
  float x=static_cast<float>(display[0]+2),y=static_cast<float>(display[1]+2),w=static_cast<float>(width),h=static_cast<float>(height);
  const auto scale=boundary_.desktop_scale();if(scale!=1.f){x=x*scale;y=y*scale;w=scale*w;h=scale*h;}
  const auto p=state_.position;
  if(p.x<x||p.y<y||x+w<=p.x||y+h<=p.y){
   const auto bounds=boundary_.global_bounds(target);
   const GuiMousePoint center{static_cast<float>(bounds[2])*.5f+static_cast<float>(bounds[0]),static_cast<float>(bounds[3])*.5f+static_cast<float>(bounds[1])};
   const auto first=boundary_.scaled_to_unscaled(center);
   state_.offset.x=(state_.position.x-first.x)+state_.offset.x;state_.offset.y=(state_.position.y-first.y)+state_.offset.y;
   boundary_.warp_cursor(boundary_.scaled_to_unscaled(center));
  }else if(state_.keep_cursor_visible&&(state_.offset.x!=0.f||state_.offset.y!=0.f)){
   const GuiMousePoint corrected{p.x+state_.offset.x,state_.offset.y+p.y};
   if(x<=corrected.x&&y<=corrected.y&&corrected.x<x+w&&corrected.y<y+h){boundary_.warp_cursor(corrected);state_.offset={0.f,0.f};}
  }
 }
 void target_changed(GuiVisibilityComponent* target,GuiMousePoint screen,std::int64_t time){
  auto* old=state_.current_target();if(target==old)return;
  const auto next=weak(target);const auto saved_modifiers=state_.modifiers;
  if(old){
   const auto previous=weak(old);buttons_changed(screen,time,0);
   if(previous&&previous->component){
    state_.target=next;auto* node=previous->component;
    const auto local=boundary_.local_position(*node,screen);boundary_.mouse_exit(*node,state_,local,time);
   }
   state_.modifiers=saved_modifiers;
  }
  state_.target=weak(next?next->component:nullptr);
  if(next&&next->component){auto* node=next->component;const auto local=boundary_.local_position(*node,screen);boundary_.mouse_enter(*node,state_,local,time);}
  boundary_.update_cursor(state_,false);buttons_changed(screen,time,saved_modifiers);
 }
 void position_changed(GuiMousePoint screen,std::int64_t time,bool force){
  if((state_.modifiers&0x70u)==0){auto* target=boundary_.find_target(state_,screen);target_changed(target,screen,time);}
  if(screen.x==state_.position.x&&screen.y==state_.position.y&&!force)return;
  state_.pending_word().exchange(0);
  if(screen.x!=-10.f||screen.y!=-10.f)state_.position=screen;
  if(auto* target=state_.current_target()){
   if((state_.modifiers&0x70u)==0){const auto local=boundary_.local_position(*target,screen);boundary_.mouse_move(*target,state_,local,time);}
   else{
    const float dx=press_float(state_.presses[0],0)-screen.x,dy=press_float(state_.presses[0],4)-screen.y;
    const bool dragged=state_.dragged||4.f<=std::sqrt(dx*dx+dy*dy);const auto extra=state_.extra;state_.dragged=dragged;
    const GuiMousePoint moved{screen.x+state_.offset.x,screen.y+state_.offset.y};
    const auto local=boundary_.local_position(*target,moved);boundary_.mouse_drag(*target,state_,local,time,extra);
    if(state_.unbounded)update_unbounded(*target);
   }
  }
  boundary_.update_cursor(state_,false);
 }
 bool buttons_changed(GuiMousePoint screen,std::int64_t time,std::uint32_t wanted){
  auto old=state_.modifiers;if(old==wanted)return false;
  if((old&0x70u)==0||(wanted&0x70u)!=0){position_changed(screen,time,false);old=state_.modifiers;}
  if(((old&0x70u)!=0)==((wanted&0x70u)!=0)){state_.modifiers=wanted;return false;}
  const auto serial=state_.event_serial;
  if((old&0x70u)!=0){
   if(auto* target=state_.current_target()){
    const auto global=boundary_.global_modifiers()&0xffffff8fu;state_.modifiers=wanted;const auto extra=state_.extra;
    const GuiMousePoint moved{screen.x+state_.offset.x,state_.offset.y+screen.y};
    const auto local=boundary_.local_position(*target,moved);boundary_.mouse_up(*target,state_,local,time,old|global,extra);
    if(serial!=state_.event_serial)return true;
   }
   unbounded(false);
  }
  state_.modifiers=wanted;
  if((wanted&0x70u)!=0){
   boundary_.increment_mouse_down_count();
   if(auto* target=state_.current_target()){
    for(unsigned i=3;i>0;--i)state_.presses[i]=state_.presses[i-1];
    auto& record=state_.presses[0];store(record,0,screen.x);store(record,4,screen.y);store(record,8,time);
    store(record,16,state_.modifiers&0x70u);record[24]=state_.source_type==1?1:0;store(record,20,boundary_.peer_identifier(*target));
    state_.dragged=false;state_.auxiliary_target.reset();const auto extra=state_.extra;const auto local=boundary_.local_position(*target,screen);
    boundary_.mouse_down(*target,state_,local,time,extra);
   }
  }
  return serial!=state_.event_serial;
 }
};
// Source ffb0d0 queued callback. The word belongs to the retained message,
// while source state and real input services remain owned by the application.
class GuiMouseSourceUpdates final:public GuiAsyncUpdateTarget {
 GuiMouseSourceState& state_;GuiMouseInputBoundary& boundary_;GuiMouseInputSource input_;GuiAsyncUpdater update_;
public:
 GuiMouseSourceUpdates(GuiMouseSourceState& state,GuiMouseInputBoundary& boundary,GuiAsyncUpdateQueue& queue)
  :state_(state),boundary_(boundary),input_(state,boundary),update_(*this,queue){
  if(state_.pending_source||state_.pending.load()!=0)throw std::logic_error("Mouse update owner must bind before posting");
  state_.pending_source=&update_.pending_word();
 }
 ~GuiMouseSourceUpdates()override{update_.cancel();if(state_.pending_source==&update_.pending_word()){state_.pending.store(0);state_.pending_source=nullptr;}}
 GuiMouseSourceUpdates(const GuiMouseSourceUpdates&)=delete;
 GuiMouseSourceUpdates& operator=(const GuiMouseSourceUpdates&)=delete;
 GuiMouseInputSource& input()noexcept{return input_;}
 void trigger(){update_.trigger();}
 void cancel()noexcept{update_.cancel();}
 void handle_async_update()override{const auto now=boundary_.current_time_ms();input_.position_changed(state_.position,std::max(state_.last_time,now),true);}
};
}
