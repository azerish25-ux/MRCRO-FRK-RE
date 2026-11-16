#include "mfrk/ui_slider_curves.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <fstream>
#include <iostream>
using namespace mfrk::ui;
struct Event {std::uint32_t kind;std::uint64_t a=0,b=0,c=0;bool operator==(const Event&)const=default;};
struct Boundary:SliderValueBoundary {
 std::int32_t count;std::vector<Event> events;
 explicit Boundary(std::int32_t n):count(n){}
 std::int32_t item_count(std::uint32_t)override{events.push_back({0});return count;}
 std::uint32_t selected_id()override{return 42;}
 void index(std::uint32_t value)override{events.push_back({1,value});}
 void set(std::uint32_t id,float value,bool notify)override{events.push_back({2,id,std::bit_cast<std::uint32_t>(value),std::uint64_t(notify)});}
 void mapped(double id,double value)override{events.push_back({3,std::bit_cast<std::uint64_t>(id),std::bit_cast<std::uint64_t>(value)});}
 void normalized(double value)override{events.push_back({4,std::bit_cast<std::uint64_t>(value)});}
 void id_value(std::uint32_t id,float value)override{events.push_back({5,id,std::bit_cast<std::uint32_t>(value)});}
};
int main(int argc,char**argv)try{
 static_assert(std::endian::native==std::endian::little);if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=141844)return 2;
 std::vector<std::uint8_t> bytes(141844);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"SLCURV01",8))return 2;
 if(scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x63,0x26,0x54,0x02,0x62,0x9e,0x99,0xca,0x33,0x75,0x0a,0x0a,0xe2,0x76,0x1b,0xf4,0xa5,0x3b,0xc3,0x72,0xb4,0xc2,0x36,0xb5,0x69,0x48,0x40,0xb7,0x5b,0xbc,0x38,0xee})return 2;
 std::size_t position=8;auto read=[&]<class T>(){if(position>bytes.size()||sizeof(T)>bytes.size()-position)throw std::runtime_error("truncated");T value;std::memcpy(&value,bytes.data()+position,sizeof value);position+=sizeof value;return value;};auto scalar=[&](){return std::bit_cast<float>(read.operator()<std::uint32_t>());};
 if(read.operator()<std::uint32_t>()!=1818||read.operator()<std::uint32_t>()!=840)return 2;
 auto definition=[&](){SliderCurveDefinition d{static_cast<SliderCurveKind>(read.operator()<std::uint8_t>()),{},{}};for(auto& x:d.fields)x=scalar();const auto count=read.operator()<std::uint16_t>();if(count>4096)throw std::runtime_error("table size");for(unsigned i=0;i<count;++i)d.table.push_back(scalar());return d;};
 unsigned curve_differences=0;for(unsigned i=0;i<1818;++i){auto d=definition();const auto direction=read.operator()<std::uint8_t>();const auto value=scalar();const auto expected=read.operator()<std::uint32_t>();auto curve=make_slider_curve(d);if(!curve||direction>1)return 2;const auto result=direction?curve->inverse(value):curve->forward(value);if(std::bit_cast<std::uint32_t>(result)!=expected){++curve_differences;std::cerr<<"curve "<<i<<" kind "<<unsigned(d.kind)<<" direction "<<unsigned(direction)<<" value "<<std::bit_cast<std::uint32_t>(value)<<" actual "<<std::bit_cast<std::uint32_t>(result)<<" expected "<<expected<<'\n';}}
 unsigned differences=0;
 for(unsigned ordinal=0;ordinal<840;++ordinal){
  auto d=definition();auto curve=make_slider_curve(d);if(!curve)return 2;const auto value=scalar();const auto id=read.operator()<std::uint32_t>();const auto count=read.operator()<std::int32_t>();const auto from=scalar(),to=scalar(),offset=scalar();const auto precision=read.operator()<std::int32_t>(),fine=read.operator()<std::int32_t>(),mode=read.operator()<std::int32_t>();const auto flags=read.operator()<std::uint8_t>();const auto expected_value=read.operator()<std::uint32_t>();const auto events=read.operator()<std::uint8_t>();if(flags>127||events>6)return 2;
  std::vector<Event> expected;
  for(unsigned i=0;i<events;++i){Event e{read.operator()<std::uint8_t>()};switch(e.kind){case 0:break;case 1:e.a=read.operator()<std::uint32_t>();break;case 2:e.a=read.operator()<std::uint32_t>();e.b=read.operator()<std::uint32_t>();e.c=read.operator()<std::uint8_t>();break;case 3:e.a=read.operator()<std::uint64_t>();e.b=read.operator()<std::uint64_t>();break;case 4:e.a=read.operator()<std::uint64_t>();break;case 5:e.a=read.operator()<std::uint32_t>();e.b=read.operator()<std::uint32_t>();break;default:return 2;}expected.push_back(e);}
  SliderValueInfo info{*curve,from,to,precision,fine,bool(flags&2),bool(flags&4)};SliderValueConfig config{bool(flags&1),mode,offset,bool(flags&16),bool(flags&32),bool(flags&64)};Boundary boundary(count);
  const auto result=slider_write(config,info,id,value,flags&8,boundary);const bool bad=std::bit_cast<std::uint32_t>(result)!=expected_value||boundary.events!=expected;differences+=bad;if(bad&&differences==1)std::cerr<<"first writer difference "<<ordinal<<'\n';
 }
 if(position!=bytes.size())return 2;
 if(make_slider_curve("unknown")||!make_slider_curve(".?AVLinearPotardMapping@MappingTools@Arturia@@"))return 3;
 std::cout<<"{\"curve_cases\":1818,\"curve_differences\":"<<curve_differences<<",\"writer_cases\":840,\"differences\":"<<differences<<"}\n";return differences||curve_differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what();return 2;}
