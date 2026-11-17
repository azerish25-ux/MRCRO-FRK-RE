#include "mfrk/ui_component_mouse.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
using Event=std::array<std::uint32_t,24>;
std::uint32_t bits(float v){return std::bit_cast<std::uint32_t>(v);}
struct Driver;
struct Listener:GuiComponentMouseListener {Driver* driver=nullptr;unsigned id=0;void mouse_event(GuiComponentMouseKind,const GuiComponentMouseEvent&)override;};
struct Driver:GuiComponentMouseButtonBoundary,GuiPointerGeometryBoundary {
 std::array<std::uint32_t,11> c;std::array<GuiVisibilityComponent,3> nodes;std::array<std::shared_ptr<GuiComponentLifetime>,3> guards;std::array<GuiComponentMouseListeners,3> registrations;std::array<Listener,10> listeners;std::vector<GuiComponentMouseListener*> globals;GuiMouseSourceState source;std::vector<Event> events;bool mutated=false,unblock=false;std::uint32_t global;
 explicit Driver(std::array<std::uint32_t,11> config):c(config),global(c[8]){
  for(unsigned i=0;i<10;++i){listeners[i].driver=this;listeners[i].id=i+3;}
  for(unsigned i=0;i<3;++i){auto& n=nodes[i];n.focus_flags=static_cast<std::uint8_t>(c[2]);n.interaction_flags=static_cast<std::uint8_t>(c[3]);n.flags=2;n.bounds={static_cast<std::int32_t>(5+i),static_cast<std::int32_t>(7+i),100,80};n.parent=i<2?&nodes[i+1]:nullptr;n.mouse_listeners=&registrations[i];registrations[i].deep_count=static_cast<std::int32_t>(c[4+i]);registrations[i].listeners={&listeners[i*2],&listeners[i*2+1]};guards[i]=n.lifetime_guard();}
  source.modifiers=c[7];globals={&listeners[6],&listeners[7]};
 }
 void mutate(unsigned id){
  if(mutated||id!=c[9])return;
  mutated=true;
  switch(c[10]){case 1:nodes[0].retire();break;case 2:nodes[1].retire();break;case 3:globals.resize(1);break;case 4:registrations[0].listeners.resize(1);break;case 5:registrations[1].deep_count=0;break;case 6:nodes[0].parent=&nodes[2];break;case 7:source.modifiers=0xf5;global=0xa5;break;default:break;}
 }
 void simple(std::uint32_t kind){events.push_back(Event{kind,nodes[0].interaction_flags});}
 void observe(unsigned id,GuiComponentMouseKind kind,const GuiComponentMouseEvent& e){
  const Event event{0,id,bits(e.position.x),bits(e.position.y),static_cast<std::uint32_t>(e.integer_position[0]),static_cast<std::uint32_t>(e.integer_position[1]),e.modifiers,e.extra[0],e.extra[1],e.extra[2],e.extra[3],e.extra[4],bits(e.mouse_down_position.x),bits(e.mouse_down_position.y),static_cast<std::uint32_t>(e.component-&nodes[0]),static_cast<std::uint32_t>(e.original_component-&nodes[0]),static_cast<std::uint32_t>(e.time),static_cast<std::uint32_t>(static_cast<std::uint64_t>(e.time)>>32),static_cast<std::uint32_t>(e.mouse_down_time),static_cast<std::uint32_t>(static_cast<std::uint64_t>(e.mouse_down_time)>>32),e.source==&source,static_cast<std::uint32_t>(e.clicks)|(e.dragged?256u:0u),nodes[0].interaction_flags,static_cast<std::uint32_t>(kind)};events.push_back(event);mutate(id);
 }
 bool modal_blocked(GuiVisibilityComponent&)override{simple(1);return c[1]!=0&&!(unblock&&mutated);}
 void repaint(GuiVisibilityComponent&)override{simple(2);}
 std::uint32_t global_mouse_modifiers()override{return global;}
 void component_mouse_event(GuiVisibilityComponent&,GuiComponentMouseKind kind,const GuiComponentMouseEvent& e)override{observe(0,kind,e);}
 void desktop_mouse_prepare()override{simple(3);}
 void desktop_mouse_blocked_move()override{simple(4);}
 void hide_cursor(GuiMouseSourceState&)override{simple(5);}
 void modal_input_attempt()override{simple(6);mutate(11);}
 void to_front(GuiVisibilityComponent& n,bool take_focus)override{events.push_back(Event{7,nodes[0].interaction_flags,static_cast<std::uint32_t>(&n-&nodes[0]),take_focus});mutate(12);}
 void grab_mouse_keyboard_focus(GuiVisibilityComponent&)override{simple(8);mutate(13);}
 GuiPointerPeer* peer(GuiVisibilityComponent&)override{throw std::logic_error("Unexpected peer");}
 bool valid_peer(GuiPointerPeer*)override{throw std::logic_error("Unexpected peer validity");}
 float component_scale(GuiVisibilityComponent&)override{return 1.f;}
 float desktop_scale()override{return 1.25f;}
 bool hit_test(GuiVisibilityComponent&,std::int32_t,std::int32_t)override{throw std::logic_error("Unexpected hit");}
 std::vector<GuiComponentMouseListener*>& global_mouse_listeners()override{return globals;}
};
void Listener::mouse_event(GuiComponentMouseKind kind,const GuiComponentMouseEvent& e){driver->observe(id,kind,e);}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;constexpr Identity expected={"component-mouse-buttons","CMBTN001",322476,384,3169030586u,{0x2,0xf9,0xc1,0x40,0x2e,0x29,0xc2,0xda,0xf1,0xa3,0xb8,0xc6,0xad,0x97,0xac,0x23,0x1c,0x97,0xae,0x13,0x6,0x45,0xe9,0xc6,0x7,0xf3,0x88,0x29,0x6,0x3e,0xaa,0x60}};auto reader=Reader::file(argv[1],expected);unsigned differences=0,event_count=0;
 for(unsigned i=0;i<expected.count;++i){std::array<std::uint32_t,11> config;for(auto& v:config)v=reader.u32();const GuiMousePoint point{reader.f32(),reader.f32()};const auto time_low=reader.u32(),time_high=reader.u32();const auto time=std::bit_cast<std::int64_t>(static_cast<std::uint64_t>(time_low)|static_cast<std::uint64_t>(time_high)<<32);std::array<std::uint32_t,32> presses;for(auto& v:presses)v=reader.u32();GuiMouseExtra extra;for(auto& v:extra)v=reader.u32();const auto last_low=reader.u32(),last_high=reader.u32();const auto last_time=std::bit_cast<std::int64_t>(static_cast<std::uint64_t>(last_low)|static_cast<std::uint64_t>(last_high)<<32);const bool dragged=reader.flag(),unblock=reader.flag();const auto up_modifiers=reader.u32();std::array<std::uint32_t,5> state;for(auto& v:state)v=reader.u32();const auto count=reader.u32();std::vector<Event> events(count);for(auto& event:events)for(auto& v:event)v=reader.u32();Driver driver(config);std::memcpy(driver.source.presses.data(),presses.data(),128);driver.source.last_time=last_time;driver.source.dragged=dragged;driver.unblock=unblock;unsigned clicks=0;
 if(config[0]==3)gui_component_mouse_down(driver.nodes[0],driver.source,point,time,extra,driver);
 else if(config[0]==4)gui_component_mouse_drag(driver.nodes[0],driver.source,point,time,extra,driver,driver);
 else if(config[0]==5)gui_component_mouse_up(driver.nodes[0],driver.source,point,time,up_modifiers,extra,driver,driver);
 else clicks=gui_component_mouse_clicks(driver.source);
 const std::array<std::uint32_t,5> actual{driver.nodes[0].interaction_flags,driver.guards[0]->component!=nullptr,driver.guards[1]->component!=nullptr,driver.guards[2]->component!=nullptr,clicks};if(actual!=state||driver.events!=events){if(differences++<5){std::cerr<<"Component mouse mismatch "<<i<<" state="<<(actual!=state)<<" events="<<driver.events.size()<<'/'<<events.size()<<'\n';for(unsigned j=0;j<std::min(driver.events.size(),events.size());++j)if(driver.events[j]!=events[j]){std::cerr<<"event "<<j<<'\n';for(unsigned k=0;k<24;++k)if(driver.events[j][k]!=events[j][k])std::cerr<<k<<' '<<driver.events[j][k]<<'/'<<events[j][k]<<'\n';break;}}}event_count+=count;}
 reader.end();if(event_count!=2351)return 2;std::cout<<"{\"cases\":"<<expected.count<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
