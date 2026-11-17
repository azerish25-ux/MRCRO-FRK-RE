#include "mfrk/ui_knob_component.hpp"
#include "mfrk/ui_knob_visible_bounds.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
using Event=std::array<std::int32_t,8>;
struct Test;
struct Cache:GuiComponentImageCache {Test* test;unsigned index;bool invalidate_all()override;bool invalidate(const GuiDirtyBounds&)override;void release_resources()override{throw std::logic_error("Unexpected cache release");}};
struct Peer:GuiRepaintPeer {Test* test;unsigned index;GuiDirtyBounds client_bounds()override;void invalidate(const GuiDirtyBounds&)override;};
struct Provider:GuiComponentBoundsProvider {Test* test;unsigned node,index;GuiDirtyBounds bounds()override;};
struct Test:GuiRepaintBoundary {
 std::array<GuiRepaintComponent,3> components{};
 std::array<std::array<float,6>,3> transforms{};
 std::array<Cache,3> caches{};std::array<Peer,3> peers{};
 std::array<GuiDirtyBounds,3> client{};std::array<bool,3> allowed{},present{},has_providers{};
 std::array<std::array<double,4>,3> fractions{};
 std::array<std::array<Provider,3>,3> providers{};
 std::array<std::vector<GuiDirtyBounds>,3> provider_bounds;
 std::array<std::vector<GuiComponentBoundsProvider*>,3> provider_lists;
 std::vector<Event> events;
 Test(){for(unsigned i=0;i<3;++i){caches[i].test=this;caches[i].index=i;peers[i].test=this;peers[i].index=i;for(unsigned j=0;j<3;++j){providers[i][j].test=this;providers[i][j].node=i;providers[i][j].index=j;}}}
 void event(std::int32_t kind,unsigned i,std::initializer_list<std::int32_t> values={}){Event e{kind,static_cast<std::int32_t>(i)};std::copy(values.begin(),values.end(),e.begin()+2);events.push_back(e);}
 GuiDirtyBounds visible_bounds(GuiRepaintComponent& c)override{const auto i=static_cast<unsigned>(&c-components.data());event(1,i);GuiKnobVisibleBounds source(fractions[i],has_providers[i]?&provider_lists[i]:nullptr);return source.get(c.bounds);}
 GuiRepaintPeer* peer(GuiRepaintComponent& c)override{const auto i=static_cast<unsigned>(&c-components.data());event(3,i);return present[i]?&peers[i]:nullptr;}
};
bool Cache::invalidate_all(){test->event(2,index,{1});return test->allowed[index];}
bool Cache::invalidate(const GuiDirtyBounds& r){test->event(2,index,{0,r[0],r[1],r[2],r[3]});return test->allowed[index];}
GuiDirtyBounds Peer::client_bounds(){test->event(4,index);return test->client[index];}
void Peer::invalidate(const GuiDirtyBounds& r){test->event(5,index,{r[0],r[1],r[2],r[3]});}
GuiDirtyBounds Provider::bounds(){test->event(6,node,{static_cast<std::int32_t>(index)});return test->provider_bounds[node][index];}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;
 constexpr Identity expected={"component-repaint","COMPRP03",228076,384,550752133u,{0xba,0xb5,0xe7,0x5,0x89,0x4,0xb7,0xc9,0xaa,0x9c,0x42,0xae,0xc9,0x5,0xf2,0xdf,0x23,0x2e,0xf1,0xda,0x44,0xda,0x5e,0x3a,0xa7,0x40,0x79,0x76,0xa1,0x95,0xcc,0xae}};
 auto reader=Reader::file(argv[1],expected);unsigned differences=0,event_count=0;
 for(unsigned i=0;i<expected.count;++i){
  GuiDirtyBounds request;for(auto& v:request)v=reader.i32();const auto entry=reader.u32();const bool immediate=reader.flag();Test test;
  for(unsigned j=0;j<3;++j){auto& c=test.components[j];c.flags=static_cast<std::uint8_t>(reader.u32());const bool parent=reader.flag(),cache=reader.flag();test.allowed[j]=reader.flag();test.present[j]=reader.flag();const bool transform=reader.flag();for(auto& v:c.bounds)v=reader.i32();for(auto& v:test.client[j])v=reader.i32();for(auto& v:test.transforms[j])v=reader.f32();for(auto& v:test.fractions[j])v=reader.f64();const auto count=reader.u32();test.has_providers[j]=count!=0xffffffffu;if(test.has_providers[j]){if(count>3)throw std::runtime_error("Bounds provider fixture count");for(unsigned k=0;k<count;++k){GuiDirtyBounds bounds;for(auto& v:bounds)v=reader.i32();test.provider_bounds[j].push_back(bounds);test.provider_lists[j].push_back(&test.providers[j][k]);}}c.parent=j<2&&parent?&test.components[j+1]:nullptr;c.cached_image=cache?&test.caches[j]:nullptr;c.transform=transform?&test.transforms[j]:nullptr;}
  GuiDirtyBounds native_request;for(auto& v:native_request)v=reader.i32();const auto count=reader.u32();std::vector<Event> native_events(count);for(auto& e:native_events)for(auto& v:e)v=reader.i32();
  if(entry==2)gui_component_full_repaint(test.components[0],test);else if(entry==1)gui_component_propagate_repaint(test.components[0],request,immediate,test);else gui_component_repaint(test.components[0],request,test);
  if(request!=native_request||test.events!=native_events){if(differences++<4){std::cerr<<"Component repaint mismatch "<<i<<" request="<<(request!=native_request)<<" events="<<test.events.size()<<'/'<<native_events.size()<<'\n';for(unsigned j=0;j<std::min(test.events.size(),native_events.size());++j)if(test.events[j]!=native_events[j]){std::cerr<<" event "<<j<<" actual:";for(auto v:test.events[j])std::cerr<<' '<<v;std::cerr<<" expected:";for(auto v:native_events[j])std::cerr<<' '<<v;std::cerr<<'\n';break;}}}event_count+=count;
 }
 reader.end();if(event_count!=1847)return 2;std::cout<<"{\"cases\":"<<expected.count<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
