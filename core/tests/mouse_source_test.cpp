#include "mfrk/ui_mouse_source.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
using Event=std::array<std::uint32_t,20>;
std::uint32_t word(std::int32_t v){return static_cast<std::uint32_t>(v);}
std::uint32_t bits(float v){return std::bit_cast<std::uint32_t>(v);}
struct Queue:GuiAsyncUpdateQueue {bool post(std::shared_ptr<GuiAsyncUpdateMessage>)override{throw std::logic_error("Unexpected mouse queue post");}};
struct Driver:GuiMouseInputBoundary {
 std::array<std::int32_t,20> c;
 std::array<float,12> f;
 std::array<std::int32_t,12> geometry;
 std::array<std::int64_t,3> times;
 GuiMouseSourceState state;std::array<GuiVisibilityComponent,2> nodes;Queue queue;
 GuiMouseSourceUpdates updates{state,*this,queue};
 std::vector<Event> events;bool mutated=false;std::uint32_t clicks=0;
 Driver(const std::array<std::int32_t,20>& config,const std::array<float,12>& fields,const std::array<std::int32_t,12>& g,const std::array<std::int64_t,3>& t):c(config),f(fields),geometry(g),times(t){for(auto& n:nodes)n.lifetime_guard();}
 std::int32_t index(GuiVisibilityComponent* node){if(!node)return -1;for(unsigned i=0;i<2;++i)if(node==&nodes[i])return static_cast<std::int32_t>(i);throw std::logic_error("Mouse component identity");}
 GuiVisibilityComponent* node(std::int32_t i){return i<0?nullptr:&nodes[static_cast<unsigned>(i)];}
 void event(std::uint32_t kind,std::int32_t id,std::initializer_list<std::uint32_t> values={}){Event e{kind,word(id)};std::copy(values.begin(),values.end(),e.begin()+2);events.push_back(e);}
 void mutate(std::uint32_t kind,std::int32_t id){
  if(mutated||kind!=static_cast<std::uint32_t>(c[15]))return;
  mutated=true;const auto action=c[16];
  if((action==1||action==2)&&id>=0)nodes[static_cast<unsigned>(action==1?id:1-id)].retire();
  else if(action==3)state.modifiers=static_cast<std::uint32_t>(c[17]);else if(action==4)++state.event_serial;else if(action==5)state.offset={-2.5f,3.f};else if(action==6)state.pending_word().store(1);
 }
 void mouse(std::uint32_t kind,GuiVisibilityComponent& target,GuiMousePoint p,std::int64_t time,std::uint32_t up=0,const GuiMouseExtra& extra={}){
  const auto id=index(&target);const auto tb=std::bit_cast<std::uint64_t>(time);
  event(kind,id,{bits(p.x),bits(p.y),static_cast<std::uint32_t>(tb),static_cast<std::uint32_t>(tb>>32),state.modifiers,word(index(state.current_target())),state.event_serial,state.dragged,state.unbounded,state.keep_cursor_visible,state.pending_word().load(),up,extra[0],extra[1],extra[2],extra[3],extra[4]});mutate(kind,id);
 }
 GuiVisibilityComponent* find_target(GuiMouseSourceState&,GuiMousePoint p)override{event(9,-1,{bits(p.x),bits(p.y)});return node(c[4]);}
 GuiMousePoint local_position(GuiVisibilityComponent& target,GuiMousePoint p)override{const auto id=index(&target);event(8,id,{bits(p.x),bits(p.y)});return {p.x-f[6+static_cast<unsigned>(id)*2],p.y-f[7+static_cast<unsigned>(id)*2]};}
 std::uint32_t global_modifiers()override{return static_cast<std::uint32_t>(c[14]);}
 std::uint32_t peer_identifier(GuiVisibilityComponent& target)override{const auto id=index(&target);event(10,id);return c[18+static_cast<unsigned>(id)]?42u+static_cast<unsigned>(id)*35u:0u;}
 void increment_mouse_down_count()override{++clicks;}
 void mouse_enter(GuiVisibilityComponent& n,GuiMouseSourceState&,GuiMousePoint p,std::int64_t t)override{mouse(1,n,p,t);}
 void mouse_exit(GuiVisibilityComponent& n,GuiMouseSourceState&,GuiMousePoint p,std::int64_t t)override{mouse(2,n,p,t);}
 void mouse_move(GuiVisibilityComponent& n,GuiMouseSourceState&,GuiMousePoint p,std::int64_t t)override{mouse(3,n,p,t);}
 void mouse_drag(GuiVisibilityComponent& n,GuiMouseSourceState&,GuiMousePoint p,std::int64_t t,const GuiMouseExtra& e)override{mouse(4,n,p,t,0,e);}
 void mouse_down(GuiVisibilityComponent& n,GuiMouseSourceState&,GuiMousePoint p,std::int64_t t,const GuiMouseExtra& e)override{mouse(5,n,p,t,0,e);}
 void mouse_up(GuiVisibilityComponent& n,GuiMouseSourceState&,GuiMousePoint p,std::int64_t t,std::uint32_t mods,const GuiMouseExtra& e)override{mouse(6,n,p,t,mods,e);}
 void update_cursor(GuiMouseSourceState&,bool force)override{event(7,-1,{force,state.modifiers,word(index(state.current_target())),state.pending_word().load()});}
 GuiDirtyBounds global_bounds(GuiVisibilityComponent& target)override{const auto id=index(&target);event(11,id);GuiDirtyBounds r;std::copy_n(geometry.begin()+id*4,4,r.begin());return r;}
 GuiDirtyBounds display_bounds(const GuiDirtyBounds& r)override{event(12,-1,{word(r[0]),word(r[1]),word(r[2]),word(r[3])});GuiDirtyBounds result;std::copy_n(geometry.begin()+8,4,result.begin());return result;}
 float desktop_scale()override{return f[10];}
 GuiMousePoint scaled_to_unscaled(GuiMousePoint p)override{event(13,-1,{bits(p.x),bits(p.y)});return {p.x*f[11],p.y*f[11]};}
 GuiMousePoint unscaled_to_scaled(GuiMousePoint p)override{event(14,-1,{bits(p.x),bits(p.y)});const auto inverse=1.f/f[11];return {p.x*inverse,p.y*inverse};}
 void warp_cursor(GuiMousePoint p)override{event(15,-1,{bits(p.x),bits(p.y)});}
 std::int64_t current_time_ms()override{const auto t=std::bit_cast<std::uint64_t>(times[2]);event(16,-1,{static_cast<std::uint32_t>(t),static_cast<std::uint32_t>(t>>32)});return times[2];}
 std::array<std::uint32_t,21> snapshot(std::uint32_t result){const auto t=std::bit_cast<std::uint64_t>(state.last_time);return {bits(state.position.x),bits(state.position.y),bits(state.offset.x),bits(state.offset.y),state.modifiers,state.source_type,state.event_serial,state.unbounded,state.keep_cursor_visible,state.dragged,state.pending_word().load(),word(index(state.current_target())),word(index(state.auxiliary_target?state.auxiliary_target->component:nullptr)),state.target!=nullptr,state.auxiliary_target!=nullptr,nodes[0].lifetime_guard()->component!=nullptr,nodes[1].lifetime_guard()->component!=nullptr,clicks,static_cast<std::uint32_t>(t),static_cast<std::uint32_t>(t>>32),result};}
};
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;constexpr Identity expected={"mouse-source","MOUSES01",326188,384,2151730327u,{0xa1,0xce,0x6,0x8b,0xe5,0x30,0x26,0xa8,0xbe,0x97,0x0,0x0,0x85,0x9c,0x9b,0xb6,0x6,0x28,0xcd,0x59,0x39,0xa5,0xa3,0xdf,0x3,0x27,0x95,0xb7,0xa9,0x2d,0xaf,0x8a}};auto reader=Reader::file(argv[1],expected);unsigned differences=0,event_count=0;
 for(unsigned i=0;i<expected.count;++i){
  std::array<std::int32_t,20> c;for(auto& v:c)v=reader.i32();std::array<float,12> f;for(auto& v:f)v=reader.f32();std::array<std::int32_t,12> geometry;for(auto& v:geometry)v=reader.i32();std::array<std::int64_t,3> times;for(auto& v:times)v=std::bit_cast<std::int64_t>(reader.u64());Driver driver(c,f,geometry,times);auto& state=driver.state;
  state.position={f[0],f[1]};state.offset={f[4],f[5]};state.modifiers=static_cast<std::uint32_t>(c[5]);state.source_type=static_cast<std::uint32_t>(c[7]);state.event_serial=static_cast<std::uint32_t>(c[8]);state.unbounded=c[9];state.keep_cursor_visible=c[10];state.dragged=c[11];state.pending_word().store(static_cast<std::uint32_t>(c[12]));state.last_time=times[1];for(auto& v:state.extra)v=reader.u32();std::array<std::uint32_t,32> initial;for(auto& v:initial)v=reader.u32();std::memcpy(state.presses.data(),initial.data(),128);if(c[1]>=0)state.target=driver.node(c[1])->lifetime_guard();if(c[2]>=0)state.auxiliary_target=driver.node(c[2])->lifetime_guard();
  std::array<std::uint32_t,21> native;for(auto& v:native)v=reader.u32();std::array<std::uint32_t,32> native_presses;for(auto& v:native_presses)v=reader.u32();const auto count=reader.u32();std::vector<Event> native_events(count);for(auto& e:native_events)for(auto& v:e)v=reader.u32();auto& input=driver.updates.input();std::uint32_t result=0xffffffffu;
  if(c[0]==0)input.position_changed({f[2],f[3]},times[0],c[13]!=0);else if(c[0]==1)input.target_changed(driver.node(c[3]),{f[2],f[3]},times[0]);else if(c[0]==2)result=input.buttons_changed({f[2],f[3]},times[0],static_cast<std::uint32_t>(c[6]));else if(c[0]==3)driver.updates.handle_async_update();else if(c[0]==4)input.unbounded(c[6]!=0);else input.update_unbounded(*driver.node(c[3]));
  const auto actual=driver.snapshot(result);std::array<std::uint32_t,32> actual_presses;std::memcpy(actual_presses.data(),state.presses.data(),128);
  if(actual!=native||actual_presses!=native_presses||driver.events!=native_events){if(differences++<5){std::cerr<<"Mouse source mismatch "<<i<<" entry="<<c[0]<<" state="<<(actual!=native)<<" presses="<<(actual_presses!=native_presses)<<" events="<<driver.events.size()<<'/'<<native_events.size()<<'\n';for(unsigned n=0;n<21;++n)if(actual[n]!=native[n])std::cerr<<" state "<<n<<' '<<actual[n]<<'/'<<native[n]<<'\n';for(unsigned j=0;j<std::min(driver.events.size(),native_events.size());++j)if(driver.events[j]!=native_events[j]){std::cerr<<" event "<<j<<" actual:";for(auto v:driver.events[j])std::cerr<<' '<<v;std::cerr<<" expected:";for(auto v:native_events[j])std::cerr<<' '<<v;std::cerr<<'\n';break;}}}event_count+=count;
 }
 reader.end();if(event_count!=1370)return 2;std::cout<<"{\"cases\":"<<expected.count<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
