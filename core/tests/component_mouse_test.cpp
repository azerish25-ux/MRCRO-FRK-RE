#include "mfrk/ui_component_mouse.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
using Event=std::array<std::uint32_t,24>;
std::uint32_t bits(float v){return std::bit_cast<std::uint32_t>(v);}
struct Driver;
struct Listener:GuiComponentMouseListener {Driver* driver=nullptr;unsigned id=0;void mouse_event(GuiComponentMouseKind,const GuiComponentMouseEvent&)override;};
struct Driver:GuiComponentMouseBoundary {
 std::array<std::uint32_t,11> c;std::array<GuiVisibilityComponent,3> nodes;std::array<std::shared_ptr<GuiComponentLifetime>,3> guards;std::array<GuiComponentMouseListeners,3> registrations;std::array<Listener,10> listeners;std::vector<GuiComponentMouseListener*> globals;GuiMouseSourceState source;std::vector<Event> events;bool mutated=false;std::uint32_t global;
 explicit Driver(std::array<std::uint32_t,11> config):c(config),global(c[8]){
  for(unsigned i=0;i<10;++i){listeners[i].driver=this;listeners[i].id=i+3;}
  for(unsigned i=0;i<3;++i){auto& n=nodes[i];n.focus_flags=static_cast<std::uint8_t>(c[2]);n.interaction_flags=static_cast<std::uint8_t>(c[3]);n.parent=i<2?&nodes[i+1]:nullptr;n.mouse_listeners=&registrations[i];registrations[i].deep_count=static_cast<std::int32_t>(c[4+i]);registrations[i].listeners={&listeners[i*2],&listeners[i*2+1]};guards[i]=n.lifetime_guard();}
  source.modifiers=c[7];globals={&listeners[6],&listeners[7]};
 }
 void mutate(unsigned id){
  if(mutated||id!=c[9])return;
  mutated=true;
  switch(c[10]){case 1:nodes[0].retire();break;case 2:nodes[1].retire();break;case 3:globals.resize(1);break;case 4:registrations[0].listeners.resize(1);break;case 5:registrations[1].deep_count=0;break;case 6:nodes[0].parent=&nodes[2];break;case 7:source.modifiers=0xf5;global=0xa5;break;default:break;}
 }
 void simple(std::uint32_t kind){events.push_back(Event{kind,nodes[0].interaction_flags});}
 void observe(unsigned id,const GuiComponentMouseEvent& e){
  const Event event{0,id,bits(e.position.x),bits(e.position.y),static_cast<std::uint32_t>(e.integer_position[0]),static_cast<std::uint32_t>(e.integer_position[1]),e.modifiers,e.extra[0],e.extra[1],e.extra[2],e.extra[3],e.extra[4],bits(e.mouse_down_position.x),bits(e.mouse_down_position.y),static_cast<std::uint32_t>(e.component-&nodes[0]),static_cast<std::uint32_t>(e.original_component-&nodes[0]),static_cast<std::uint32_t>(e.time),static_cast<std::uint32_t>(static_cast<std::uint64_t>(e.time)>>32),static_cast<std::uint32_t>(e.mouse_down_time),static_cast<std::uint32_t>(static_cast<std::uint64_t>(e.mouse_down_time)>>32),e.source==&source,static_cast<std::uint32_t>(e.clicks)|(e.dragged?256u:0u),nodes[0].interaction_flags};events.push_back(event);mutate(id);
 }
 bool modal_blocked(GuiVisibilityComponent&)override{simple(1);return c[1]!=0;}
 void repaint(GuiVisibilityComponent&)override{simple(2);}
 std::uint32_t global_mouse_modifiers()override{return global;}
 void component_mouse_event(GuiVisibilityComponent&,GuiComponentMouseKind,const GuiComponentMouseEvent& e)override{observe(0,e);}
 void desktop_mouse_prepare()override{simple(3);}
 void desktop_mouse_blocked_move()override{simple(4);}
 void hide_cursor(GuiMouseSourceState&)override{simple(5);}
 std::vector<GuiComponentMouseListener*>& global_mouse_listeners()override{return globals;}
};
void Listener::mouse_event(GuiComponentMouseKind,const GuiComponentMouseEvent& e){driver->observe(id,e);}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;constexpr Identity expected={"component-mouse-hover","CMHOV001",239052,288,2204499011u,{0x2e,0x7d,0xf0,0xfa,0xab,0xe4,0xca,0x98,0x22,0xe2,0x7b,0xa3,0x68,0x34,0x15,0xd6,0x3,0xe0,0x69,0x9e,0x7,0x73,0x5a,0x10,0x2c,0xdc,0xcd,0x53,0x75,0x78,0xf2,0xb6}};auto reader=Reader::file(argv[1],expected);unsigned differences=0,event_count=0;
 for(unsigned i=0;i<expected.count;++i){std::array<std::uint32_t,11> config;for(auto& v:config)v=reader.u32();const GuiMousePoint point{reader.f32(),reader.f32()};const auto time_low=reader.u32(),time_high=reader.u32();const auto time=std::bit_cast<std::int64_t>(static_cast<std::uint64_t>(time_low)|static_cast<std::uint64_t>(time_high)<<32);std::array<std::uint32_t,4> state;for(auto& v:state)v=reader.u32();const auto count=reader.u32();std::vector<Event> events(count);for(auto& event:events)for(auto& v:event)v=reader.u32();Driver driver(config);gui_component_mouse_hover(driver.nodes[0],driver.source,point,time,static_cast<GuiComponentMouseKind>(config[0]),driver);const std::array<std::uint32_t,4> actual{driver.nodes[0].interaction_flags,driver.guards[0]->component!=nullptr,driver.guards[1]->component!=nullptr,driver.guards[2]->component!=nullptr};if(actual!=state||driver.events!=events){if(differences++<5){std::cerr<<"Component mouse mismatch "<<i<<" state="<<(actual!=state)<<" events="<<driver.events.size()<<'/'<<events.size()<<'\n';for(unsigned j=0;j<std::min(driver.events.size(),events.size());++j)if(driver.events[j]!=events[j]){std::cerr<<"event "<<j<<'\n';for(unsigned k=0;k<24;++k)if(driver.events[j][k]!=events[j][k])std::cerr<<k<<' '<<driver.events[j][k]<<'/'<<events[j][k]<<'\n';break;}}}event_count+=count;}
 reader.end();if(event_count!=2250)return 2;std::cout<<"{\"cases\":"<<expected.count<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
