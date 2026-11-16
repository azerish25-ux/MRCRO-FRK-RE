#include "mfrk/ui_slider_drag.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <fstream>
#include <iostream>
using namespace mfrk::ui;
struct Event {std::uint32_t kind,a=0,b=0,c=0,d=0;bool operator==(const Event&)const=default;};
struct Boundary:SliderDragBoundary {
 SliderDragConfig& config;std::array<std::array<std::int32_t,2>,4> points;std::size_t index=0;
 bool echo,flip;float writer;unsigned layout;bool flag20,flag21;std::vector<Event> events;
 Boundary(SliderDragConfig& c,decltype(points) p,bool e,bool f,float w,unsigned l,bool a,bool b):config(c),points(p),echo(e),flip(f),writer(w),layout(l),flag20(a),flag21(b){}
 std::array<std::int32_t,2> global_position(const SliderDragEvent&)override{auto p=points[std::min(index++,points.size()-1)];events.push_back({3,std::uint32_t(p[0]),std::uint32_t(p[1])});return p;}
 MagneticResult map(float value,float delta,std::uint32_t id)override{
  events.push_back({4,std::bit_cast<std::uint32_t>(value),std::bit_cast<std::uint32_t>(delta),id});
  const std::array<MagneticZone,1> identity{{{0,1,0,1,false}}},quarter{{{0,.25f,0,1,false}}};const std::array<MagneticZone,3> center{{{0,.46f,0,.5f,false},{.46f,.54f,.5f,.5f,true},{.54f,1,.5f,1,false}}};
  const auto zones=layout==0?std::span<const MagneticZone>(identity):layout==1?std::span<const MagneticZone>(center):std::span<const MagneticZone>(quarter);
  return magnetic_delta(zones,value,delta,flag20,flag21,config.wrap,true);
 }
 std::uint32_t selected_id()override{events.push_back({5});return 42;}
 float write(float value,std::uint32_t id,bool custom,bool precise)override{events.push_back({6,std::bit_cast<std::uint32_t>(value),id,std::uint32_t(custom),std::uint32_t(precise)});if(flip)config.orientation=config.orientation==1?0:1;return echo?value:writer;}
};
int main(int argc,char**argv)try{
 static_assert(std::endian::native==std::endian::little);if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=342647)return 2;
 std::vector<std::uint8_t> bytes(342647);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"SLDRAG01",8))return 2;
 if(scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xb7,0xe0,0x7b,0xe2,0x6b,0xa5,0xe3,0x0f,0x0a,0x52,0x82,0x22,0x6b,0xff,0xd7,0xc2,0xf3,0x65,0x34,0x31,0x9d,0xe6,0xe1,0xfb,0x2e,0x8f,0x53,0x4a,0xfe,0x4c,0xa7,0x03})return 2;
 std::size_t position=8;auto read=[&]<class T>(){if(position>bytes.size()||sizeof(T)>bytes.size()-position)throw std::runtime_error("truncated");T result;std::memcpy(&result,bytes.data()+position,sizeof result);position+=sizeof result;return result;};
 auto scalar=[&](){return std::bit_cast<float>(read.operator()<std::uint32_t>());};auto boolean=[&](){auto x=read.operator()<std::uint8_t>();if(x>1)throw std::runtime_error("boolean");return bool(x);};
 auto state=[&](){const auto value=scalar();const auto id=read.operator()<std::uint32_t>();const auto sensitivity=scalar(),global=scalar(),local=scalar();const bool relative=boolean(),outside=boolean();return SliderDragState{value,id,sensitivity,global,local,relative,outside};};
 auto events=[&](){auto n=read.operator()<std::uint8_t>();if(n>16)throw std::runtime_error("events");std::vector<Event> result;for(unsigned i=0;i<n;++i){Event e{read.operator()<std::uint8_t>()};if(e.kind>6)throw std::runtime_error("event type");if(e.kind==3){e.a=read.operator()<std::uint32_t>();e.b=read.operator()<std::uint32_t>();}else if(e.kind==4){e.a=read.operator()<std::uint32_t>();e.b=read.operator()<std::uint32_t>();e.c=read.operator()<std::uint32_t>();}else if(e.kind==6){e.a=read.operator()<std::uint32_t>();e.b=read.operator()<std::uint32_t>();e.c=boolean();e.d=boolean();}result.push_back(e);}return result;};
 if(read.operator()<std::uint32_t>()!=1600||read.operator()<std::uint32_t>()!=1800)return 2;
 unsigned sensitivity_differences=0,drag_differences=0;
 for(unsigned i=0;i<1600;++i){
  SliderSensitivity p{};p.override_value=scalar();p.normal=scalar();p.fine=scalar();p.orientation=read.operator()<std::int32_t>();p.width=read.operator()<std::int32_t>();p.height=read.operator()<std::int32_t>();p.item_count=read.operator()<std::int32_t>();p.modifiers=read.operator()<std::uint32_t>();p.fine_enabled=boolean();p.enum_adjust=boolean();p.by_size=boolean();p.is_enum=boolean();const auto expected=read.operator()<std::uint32_t>();const auto source_queries=events();if(source_queries.empty()||source_queries[0].kind!=0)return 2;
  sensitivity_differences+=std::bit_cast<std::uint32_t>(slider_sensitivity(p))!=expected;
 }
 auto same=[](const SliderDragState& a,const SliderDragState& b){return a.parameter_id==b.parameter_id&&std::bit_cast<std::uint32_t>(a.value_anchor)==std::bit_cast<std::uint32_t>(b.value_anchor)&&std::bit_cast<std::uint32_t>(a.sensitivity)==std::bit_cast<std::uint32_t>(b.sensitivity)&&std::bit_cast<std::uint32_t>(a.global_anchor)==std::bit_cast<std::uint32_t>(b.global_anchor)&&std::bit_cast<std::uint32_t>(a.local_anchor)==std::bit_cast<std::uint32_t>(b.local_anchor)&&a.global_tracking==b.global_tracking&&a.outside==b.outside;};
 for(unsigned i=0;i<1800;++i){
  auto current_state=state();const auto orientation=read.operator()<std::int32_t>();const SliderDragEvent event{read.operator()<std::int32_t>(),read.operator()<std::int32_t>()};std::array<std::array<std::int32_t,2>,4> points;for(auto& p:points){p[0]=read.operator()<std::int32_t>();p[1]=read.operator()<std::int32_t>();}
  const auto current=scalar(),sensitivity=scalar(),threshold=scalar();const bool apply=boolean(),reset=boolean(),wrap=boolean(),edge=boolean(),custom=boolean(),echo=boolean();const auto writer=scalar();const bool flip=boolean();const auto layout=read.operator()<std::uint8_t>();const bool flag20=boolean(),flag21=boolean();if(layout>2)return 2;
  const auto expected=state();const auto expected_orientation=read.operator()<std::int32_t>();const auto expected_events=events();SliderDragConfig config{orientation,threshold,wrap,edge,custom};Boundary boundary(config,points,echo,flip,writer,layout,flag20,flag21);
  slider_drag(current_state,config,event,42,current,sensitivity,apply,reset,boundary);const bool bad=!same(current_state,expected)||config.orientation!=expected_orientation||boundary.events!=expected_events;drag_differences+=bad;if(bad&&drag_differences==1){auto dump=[](const SliderDragState& s){std::cerr<<std::bit_cast<std::uint32_t>(s.value_anchor)<<' '<<s.parameter_id<<' '<<std::bit_cast<std::uint32_t>(s.sensitivity)<<' '<<std::bit_cast<std::uint32_t>(s.global_anchor)<<' '<<std::bit_cast<std::uint32_t>(s.local_anchor)<<' '<<s.global_tracking<<' '<<s.outside<<'\n';};std::cerr<<"first drag difference "<<i<<'\n';dump(current_state);dump(expected);for(const auto& e:boundary.events)std::cerr<<"actual "<<e.kind<<' '<<e.a<<' '<<e.b<<' '<<e.c<<' '<<e.d<<'\n';for(const auto& e:expected_events)std::cerr<<"expected "<<e.kind<<' '<<e.a<<' '<<e.b<<' '<<e.c<<' '<<e.d<<'\n';}
 }
 if(position!=bytes.size())return 2;
 std::cout<<"{\"sensitivity_cases\":1600,\"drag_cases\":1800,\"sensitivity_differences\":"<<sensitivity_differences<<",\"drag_differences\":"<<drag_differences<<"}\n";return sensitivity_differences||drag_differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what();return 2;}
