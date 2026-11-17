#include "mfrk/ui_pointer_geometry.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
using Event=std::array<std::uint32_t,8>;
std::uint32_t w(std::int32_t value){return static_cast<std::uint32_t>(value);}
std::uint32_t bits(float value){return std::bit_cast<std::uint32_t>(value);}
struct Driver;
struct Peer:GuiPointerPeer {
 Driver* driver;unsigned id;
 GuiVisibilityComponent& root_component()override;
 GuiMousePoint local_to_global(GuiMousePoint)override;
 GuiMousePoint global_to_local(GuiMousePoint)override;
 bool contains(std::int32_t,std::int32_t,bool)override;
 std::uint32_t identifier()override{throw std::logic_error("Unexpected peer identifier");}
 GuiDirtyBounds client_bounds()override{throw std::logic_error("Unexpected client bounds");}
 void invalidate(const GuiDirtyBounds&)override{throw std::logic_error("Unexpected peer repaint");}
 bool minimized()override{throw std::logic_error("Unexpected peer visibility");}
 void set_visible(bool)override{throw std::logic_error("Unexpected peer visibility");}
 void grab_focus()override{throw std::logic_error("Unexpected peer focus");}
 bool focused()override{throw std::logic_error("Unexpected peer focus");}
};
struct Driver:GuiPointerGeometryBoundary {
 std::array<std::int32_t,9> c;std::array<float,13> f;
 std::array<GuiVisibilityComponent,5> nodes;std::array<std::array<float,6>,5> matrices{};
 std::array<float,5> scales{};std::array<bool,5> hits{};std::array<Peer,2> peers{};GuiMouseSourceState state;
 std::vector<Event> events;
 Driver(const std::array<std::int32_t,9>& config,const std::array<float,13>& fields):c(config),f(fields){for(unsigned i=0;i<2;++i){peers[i].driver=this;peers[i].id=i;}}
 GuiVisibilityComponent* node(std::int32_t i){return i<0?nullptr:&nodes[static_cast<unsigned>(i)];}
 std::int32_t index(GuiVisibilityComponent* p){if(!p)return -1;for(unsigned i=0;i<5;++i)if(p==&nodes[i])return static_cast<std::int32_t>(i);throw std::logic_error("Geometry node identity");}
 std::int32_t peer_index(GuiPointerPeer* p){if(!p)return -1;for(unsigned i=0;i<2;++i)if(p==&peers[i])return static_cast<std::int32_t>(i);throw std::logic_error("Geometry peer identity");}
 void event(std::uint32_t kind,std::int32_t id,std::initializer_list<std::uint32_t> values={}){Event e{kind,w(id)};std::copy(values.begin(),values.end(),e.begin()+2);events.push_back(e);}
 GuiPointerPeer* peer(GuiVisibilityComponent& n)override{const auto i=index(&n);event(2,i);const auto id=i==0?0u:1u;return c[5+id]?&peers[id]:nullptr;}
 bool valid_peer(GuiPointerPeer* p)override{event(7,peer_index(p));return c[4]&&p;}
 float component_scale(GuiVisibilityComponent& n)override{const auto i=index(&n);event(1,i);return scales[static_cast<unsigned>(i)];}
 float desktop_scale()override{return f[2];}
 bool hit_test(GuiVisibilityComponent& n,std::int32_t x,std::int32_t y)override{const auto i=index(&n);event(6,i,{w(x),w(y)});return hits[static_cast<unsigned>(i)];}
};
GuiVisibilityComponent& Peer::root_component(){return driver->nodes[id==0?0:4];}
GuiMousePoint Peer::local_to_global(GuiMousePoint p){driver->event(3,static_cast<std::int32_t>(id),{bits(p.x),bits(p.y)});return {p.x*2.f+driver->f[3+id*2],p.y*2.f+driver->f[4+id*2]};}
GuiMousePoint Peer::global_to_local(GuiMousePoint p){driver->event(4,static_cast<std::int32_t>(id),{bits(p.x),bits(p.y)});return {(p.x-driver->f[3+id*2])*.5f,(p.y-driver->f[4+id*2])*.5f};}
bool Peer::contains(std::int32_t x,std::int32_t y,bool children){driver->event(5,static_cast<std::int32_t>(id),{w(x),w(y),children});return driver->c[7+id]!=0;}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;constexpr Identity expected={"pointer-geometry","PTRGE001",268172,560,23386343u,{0xa5,0xda,0x18,0x1d,0xaf,0x46,0xe7,0xbf,0x89,0x51,0xeb,0xa6,0x82,0x8,0x14,0x1a,0x54,0xdb,0x8f,0x79,0x8a,0x2,0x4f,0x64,0x2c,0x23,0x49,0xa4,0x64,0xd3,0x74,0x45}};auto reader=Reader::file(argv[1],expected);unsigned differences=0,event_count=0;
 for(unsigned i=0;i<expected.count;++i){std::array<std::int32_t,9> c;for(auto& v:c)v=reader.i32();std::array<float,13> f;for(auto& v:f)v=reader.f32();Driver d(c,f);
  for(unsigned j=0;j<5;++j){auto& n=d.nodes[j];n.flags=static_cast<std::uint8_t>(reader.u32());n.parent=d.node(reader.i32());for(auto& v:n.bounds)v=reader.i32();d.scales[j]=reader.f32();const bool matrix=reader.flag();for(auto& v:d.matrices[j])v=reader.f32();n.transform=matrix?&d.matrices[j]:nullptr;}
  for(auto& v:d.hits)v=reader.flag();
  for(unsigned j=0;j<5;++j)if(d.nodes[j].parent)static_cast<GuiVisibilityComponent*>(d.nodes[j].parent)->children.push_back(&d.nodes[j]);
  d.state.peer=c[3]<0?nullptr:&d.peers[static_cast<unsigned>(c[3])];std::array<std::uint32_t,6> native;for(auto& v:native)v=reader.u32();const auto native_peer=reader.i32(),count=static_cast<std::int32_t>(reader.u32());std::vector<Event> events(static_cast<std::size_t>(count));for(auto& e:events)for(auto& v:e)v=reader.u32();std::array<std::uint32_t,6> actual{};const GuiMousePoint point{f[0],f[1]};GuiMousePoint p{};
  if(c[0]==0){p=gui_pointer_map_point(d.node(c[1]),d.node(c[2]),point,d);actual[0]=bits(p.x);actual[1]=bits(p.y);}
  else if(c[0]==1){p=gui_pointer_from_parent(*d.node(c[1]),point,d);actual[0]=bits(p.x);actual[1]=bits(p.y);}
  else if(c[0]==2)actual[0]=gui_pointer_contains(*d.node(c[1]),point,d);
  else if(c[0]==3)actual[0]=w(d.index(gui_pointer_component_at(*d.node(c[1]),point,d)));
  else if(c[0]==4)actual[0]=w(d.index(gui_mouse_find_target(d.state,point,d)));
  else if(c[0]==5){p=gui_mouse_local_position(*d.node(c[1]),point,d);actual[0]=bits(p.x);actual[1]=bits(p.y);}
  else {std::array<float,6> matrix;std::copy_n(f.begin()+7,6,matrix.begin());const auto inverse=gui_pointer_inverse(matrix);for(unsigned j=0;j<6;++j)actual[j]=bits(inverse[j]);}
  if(actual!=native||d.peer_index(d.state.peer)!=native_peer||d.events!=events){if(differences++<5){std::cerr<<"Pointer geometry mismatch "<<i<<" entry="<<c[0]<<" value="<<(actual!=native)<<" peer="<<(d.peer_index(d.state.peer)!=native_peer)<<" events="<<d.events.size()<<'/'<<events.size()<<'\n';for(unsigned j=0;j<6;++j)if(actual[j]!=native[j])std::cerr<<" value "<<j<<' '<<actual[j]<<'/'<<native[j]<<'\n';for(unsigned j=0;j<std::min(d.events.size(),events.size());++j)if(d.events[j]!=events[j]){std::cerr<<" event "<<j<<" actual:";for(auto v:d.events[j])std::cerr<<' '<<v;std::cerr<<" expected:";for(auto v:events[j])std::cerr<<' '<<v;std::cerr<<'\n';break;}}}event_count+=static_cast<unsigned>(count);
 }
 reader.end();if(event_count!=1030)return 2;std::cout<<"{\"cases\":"<<expected.count<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
