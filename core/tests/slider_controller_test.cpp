#include "mfrk/ui_slider_controller.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <fstream>
#include <iostream>
using namespace mfrk::ui;
struct Event {std::uint32_t kind;std::uint64_t a=0,b=0,c=0;bool operator==(const Event&)const=default;};
struct Identity:SliderCurve {float forward(float x)override{return x;}float inverse(float x)override{return x;}};
struct Host final:SliderControllerHost {
 std::array<std::uint32_t,55>& r;Identity identity;SliderValueInfo normal{identity,-100,100,2,4,false,false},custom{identity,-10,10,1,3,false,false};float value;std::vector<Event> events;
 explicit Host(std::array<std::uint32_t,55>& row):r(row),value(std::bit_cast<float>(r[0])){}
 void prepare(std::uint32_t)override{events.push_back({4});}
 SliderValueInfo& info(std::uint32_t,bool use_custom)override{if(use_custom)return custom;events.push_back({5});return normal;}
 void edit(std::uint32_t,std::uint32_t modifiers,bool secondary)override{events.push_back({6,modifiers,std::uint64_t(secondary)});}
 bool blocked(std::uint32_t id)override{events.push_back({7,id});return r[12];}
 float sensitivity(const SliderDragEvent& e,std::uint32_t)override{const float result=(e.modifiers&0x22)?.001f:.005f;events.push_back({8,std::bit_cast<std::uint32_t>(result)});return result;}
 std::array<std::int32_t,2> global_position(const SliderDragEvent& e)override{const auto x=e.x+100,y=e.y-50;events.push_back({9,std::uint32_t(x),std::uint32_t(y)});return {x,y};}
 std::int32_t item_count(std::uint32_t)override{events.push_back({11});return static_cast<std::int32_t>(r[1]);}
 std::uint32_t selected_id()override{return 42;}
 void index(std::uint32_t x)override{events.push_back({12,x});}
 void set(std::uint32_t id,float v,bool notify)override{events.push_back({13,id,std::bit_cast<std::uint32_t>(v),std::uint64_t(notify)});value=v;}
 void mapped(double id,double v)override{events.push_back({14,std::bit_cast<std::uint64_t>(id),std::bit_cast<std::uint64_t>(v)});}
 void normalized(double v)override{events.push_back({15,std::bit_cast<std::uint64_t>(v)});}
 void id_value(std::uint32_t id,float v)override{events.push_back({16,id,std::bit_cast<std::uint32_t>(v)});}
 bool parent_enabled()override{events.push_back({0});return r[6];}
 std::int32_t pointer_mode()override{return static_cast<std::int32_t>(r[17]);}
 void enable_unbounded_pointer()override{events.push_back({1});}
 bool phase(std::uint32_t phase,const SliderDragEvent&,std::uint32_t& id)override{events.push_back({2,phase,id,r[14]});id=r[14];return r[13];}
 float current(std::uint32_t)override{events.push_back({3,std::bit_cast<std::uint32_t>(value)});return value;}
 MagneticResult map(float v,float delta,std::uint32_t id)override{events.push_back({10,std::bit_cast<std::uint32_t>(v),std::bit_cast<std::uint32_t>(delta),id});constexpr std::array<MagneticZone,3> zones{{{0,.46f,0,.5f,false},{.46f,.54f,.5f,.5f,true},{.54f,1,.5f,1,false}}};return magnetic_delta(zones,v,delta,false,false,r[9],id==42);}
 void after_down(const SliderDragEvent&)override{events.push_back({17});}
 void after_drag(const SliderDragEvent&)override{events.push_back({18});}
 void before_up(const SliderDragEvent&,bool)override{}
 void end_edit(std::uint32_t)override{events.push_back({20});}
 void reset_timer()override{}
 void after_up(const SliderDragEvent&)override{events.push_back({19});}
};
int main(int argc,char** argv)try{
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=213372)return 2;
 std::vector<std::uint8_t> bytes(213372);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"SLCTRL02",8))return 2;
 if(scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x82,0xee,0xec,0x94,0x03,0x71,0x38,0x9a,0x39,0x02,0x1a,0x21,0x6a,0xa0,0x66,0xd8,0x57,0x66,0xf1,0xdb,0xf5,0xa1,0x22,0xa0,0x9a,0x21,0x31,0x77,0x5e,0x80,0xe7,0x53})return 2;
 std::size_t position=8;auto read=[&]<class T>(){if(position>bytes.size()||sizeof(T)>bytes.size()-position)throw std::runtime_error("truncated");T value;std::memcpy(&value,bytes.data()+position,sizeof value);position+=sizeof value;return value;};
 if(read.operator()<std::uint32_t>()!=192)return 2;
 unsigned differences=0,total_events=0;
 for(unsigned ordinal=0;ordinal<192;++ordinal){
  std::array<std::uint32_t,55> row{};for(auto& x:row)x=read.operator()<std::uint32_t>();const auto count=read.operator()<std::uint32_t>();if(count>128)return 2;total_events+=count;std::vector<Event> expected;
  for(unsigned i=0;i<count;++i)expected.push_back({read.operator()<std::uint32_t>(),read.operator()<std::uint64_t>(),read.operator()<std::uint64_t>(),read.operator()<std::uint64_t>()});
  Host host(row);SliderPointerConfig pc{bool(row[3]),bool(row[5]),bool(row[4]),bool(row[11]),bool(row[7]),row[16],bool(row[21])};SliderPointerState ps{false,0xffffffffu,.25f};SliderDragConfig dc{static_cast<std::int32_t>(row[15]),.001f,bool(row[9]),bool(row[10]),bool(row[8])};SliderDragState ds{0,0xffffffffu,0,0,0,false,false};SliderValueConfig vc{bool(row[2]),0,0,true,true,true};SliderController controller(pc,ps,dc,ds,vc,host);
  auto snapshot=[&](){return std::array<std::uint32_t,11>{std::bit_cast<std::uint32_t>(ds.value_anchor),ds.parameter_id,std::bit_cast<std::uint32_t>(ds.sensitivity),std::bit_cast<std::uint32_t>(ds.global_anchor),std::bit_cast<std::uint32_t>(ds.local_anchor),std::uint32_t(ds.global_tracking),std::uint32_t(ds.outside),std::bit_cast<std::uint32_t>(vc.offset),std::uint32_t(ps.dragged),ps.parameter_to_paint,std::bit_cast<std::uint32_t>(ps.wheel_position)};};
  const SliderDragEvent move{std::bit_cast<std::int32_t>(row[18]),std::bit_cast<std::int32_t>(row[19]),row[20]};
  controller.down({10,20,0});const auto first=snapshot();controller.drag(move);const auto second=snapshot();controller.up(move);const auto third=snapshot();
  controller.write(.37555f,row[14],row[20]&2,row[20]&32);controller.write_selected(.62555f);
  bool bad=host.events!=expected;for(unsigned i=0;i<11;++i)bad|=first[i]!=row[22+i]||second[i]!=row[33+i]||third[i]!=row[44+i];
  if(bad&&!differences){std::cerr<<"first controller difference "<<ordinal<<" events "<<host.events.size()<<'/'<<expected.size()<<'\n';for(unsigned i=0;i<std::min(host.events.size(),expected.size());++i)if(host.events[i]!=expected[i]){const auto a=host.events[i],e=expected[i];std::cerr<<"event "<<i<<" actual "<<a.kind<<' '<<a.a<<' '<<a.b<<' '<<a.c<<" expected "<<e.kind<<' '<<e.a<<' '<<e.b<<' '<<e.c<<'\n';break;}}
  differences+=bad;
 }
 if(position!=bytes.size()||total_events!=6084)return 2;
 std::cout<<"{\"cases\":192,\"events\":6084,\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what();return 2;}
