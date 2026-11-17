#include "mfrk/ui_knob_component.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
using Event=std::array<std::int32_t,8>;
struct Test;
struct Gate:GuiRepaintGate {Test* test;unsigned index;bool allow(bool)override;};
struct Peer:GuiRepaintPeer {Test* test;unsigned index;GuiDirtyBounds client_bounds()override;void invalidate(const GuiDirtyBounds&)override;};
struct Test:GuiRepaintBoundary {
 std::array<GuiRepaintComponent,3> components{};
 std::array<std::array<float,6>,3> transforms{};
 std::array<Gate,3> gates{};std::array<Peer,3> peers{};
 std::array<GuiDirtyBounds,3> visible{},client{};std::array<bool,3> allowed{},present{};
 std::vector<Event> events;
 Test(){for(unsigned i=0;i<3;++i){gates[i].test=this;gates[i].index=i;peers[i].test=this;peers[i].index=i;}}
 void event(std::int32_t kind,unsigned i,std::initializer_list<std::int32_t> values={}){Event e{kind,static_cast<std::int32_t>(i)};std::copy(values.begin(),values.end(),e.begin()+2);events.push_back(e);}
 GuiDirtyBounds visible_bounds(GuiRepaintComponent& c)override{const auto i=static_cast<unsigned>(&c-components.data());event(1,i);return visible[i];}
 GuiRepaintPeer* peer(GuiRepaintComponent& c)override{const auto i=static_cast<unsigned>(&c-components.data());event(3,i);return present[i]?&peers[i]:nullptr;}
};
bool Gate::allow(bool immediate){test->event(2,index,{immediate});return test->allowed[index];}
GuiDirtyBounds Peer::client_bounds(){test->event(4,index);return test->client[index];}
void Peer::invalidate(const GuiDirtyBounds& r){test->event(5,index,{r[0],r[1],r[2],r[3]});}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;
 constexpr Identity expected={"component-repaint","COMPRP01",157708,384,1391845557u,{0xda,0xf,0xae,0x35,0xe6,0x30,0xd7,0x17,0xbe,0xea,0x37,0x7e,0xf4,0xe6,0xb6,0x90,0xb,0x8c,0x77,0x69,0x67,0xb1,0xf6,0xbd,0x2a,0x4a,0xf5,0xe5,0x2c,0xe6,0x57,0xd0}};
 auto reader=Reader::file(argv[1],expected);unsigned differences=0,event_count=0;
 for(unsigned i=0;i<expected.count;++i){
  GuiDirtyBounds request;for(auto& v:request)v=reader.i32();const bool direct=reader.flag(),immediate=reader.flag();Test test;
  for(unsigned j=0;j<3;++j){auto& c=test.components[j];c.flags=static_cast<std::uint8_t>(reader.u32());const bool parent=reader.flag(),gate=reader.flag();test.allowed[j]=reader.flag();test.present[j]=reader.flag();const bool transform=reader.flag();for(auto& v:c.bounds)v=reader.i32();for(auto& v:test.visible[j])v=reader.i32();for(auto& v:test.client[j])v=reader.i32();for(auto& v:test.transforms[j])v=reader.f32();c.parent=j<2&&parent?&test.components[j+1]:nullptr;c.gate=gate?&test.gates[j]:nullptr;c.transform=transform?&test.transforms[j]:nullptr;}
  GuiDirtyBounds native_request;for(auto& v:native_request)v=reader.i32();const auto count=reader.u32();std::vector<Event> native_events(count);for(auto& e:native_events)for(auto& v:e)v=reader.i32();
  if(direct)gui_component_propagate_repaint(test.components[0],request,immediate,test);else gui_component_repaint(test.components[0],request,test);
  if(request!=native_request||test.events!=native_events){if(differences++<4){std::cerr<<"Component repaint mismatch "<<i<<" request="<<(request!=native_request)<<" events="<<test.events.size()<<'/'<<native_events.size()<<'\n';for(unsigned j=0;j<std::min(test.events.size(),native_events.size());++j)if(test.events[j]!=native_events[j]){std::cerr<<" event "<<j<<" actual:";for(auto v:test.events[j])std::cerr<<' '<<v;std::cerr<<" expected:";for(auto v:native_events[j])std::cerr<<' '<<v;std::cerr<<'\n';break;}}}event_count+=count;
 }
 reader.end();if(event_count!=944)return 2;std::cout<<"{\"cases\":"<<expected.count<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
