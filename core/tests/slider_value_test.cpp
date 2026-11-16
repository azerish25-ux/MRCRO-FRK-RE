#include "mfrk/ui_slider_value.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <fstream>
#include <iostream>
using namespace mfrk::ui;
struct Identity:SliderCurve {float forward(float value)override{return value;}float inverse(float value)override{return value;}};
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
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=193772)return 2;
 std::vector<std::uint8_t> bytes(193772);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"SLVAL001",8))return 2;
 if(scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xae,0x93,0xb5,0x39,0xe6,0xe4,0xc2,0xff,0x70,0x4c,0xf4,0x2a,0xc3,0x75,0x7a,0xc2,0x08,0x5a,0x41,0x72,0x7f,0xd8,0x39,0x20,0xaf,0x91,0x65,0xfc,0x4f,0x2f,0x42,0x6c})return 2;
 std::size_t position=8;auto read=[&]<class T>(){if(position>bytes.size()||sizeof(T)>bytes.size()-position)throw std::runtime_error("truncated");T value;std::memcpy(&value,bytes.data()+position,sizeof value);position+=sizeof value;return value;};auto scalar=[&](){return std::bit_cast<float>(read.operator()<std::uint32_t>());};
 if(read.operator()<std::uint32_t>()!=2400)return 2;
 Identity curve;unsigned differences=0;
 for(unsigned ordinal=0;ordinal<2400;++ordinal){
  const auto value=scalar();const auto id=read.operator()<std::uint32_t>();const auto count=read.operator()<std::int32_t>();const auto from=scalar(),to=scalar(),offset=scalar();const auto precision=read.operator()<std::int32_t>(),fine=read.operator()<std::int32_t>(),mode=read.operator()<std::int32_t>();const auto flags=read.operator()<std::uint8_t>();const auto expected_value=read.operator()<std::uint32_t>();const auto events=read.operator()<std::uint8_t>();if(flags>127||events>6)return 2;
  std::vector<Event> expected;
  for(unsigned i=0;i<events;++i){Event e{read.operator()<std::uint8_t>()};switch(e.kind){case 0:break;case 1:e.a=read.operator()<std::uint32_t>();break;case 2:e.a=read.operator()<std::uint32_t>();e.b=read.operator()<std::uint32_t>();e.c=read.operator()<std::uint8_t>();break;case 3:e.a=read.operator()<std::uint64_t>();e.b=read.operator()<std::uint64_t>();break;case 4:e.a=read.operator()<std::uint64_t>();break;case 5:e.a=read.operator()<std::uint32_t>();e.b=read.operator()<std::uint32_t>();break;default:return 2;}expected.push_back(e);}
  SliderValueInfo info{curve,from,to,precision,fine,bool(flags&2),bool(flags&4)};SliderValueConfig config{bool(flags&1),mode,offset,bool(flags&16),bool(flags&32),bool(flags&64)};Boundary boundary(count);
  const auto result=slider_write(config,info,id,value,flags&8,boundary);const bool bad=std::bit_cast<std::uint32_t>(result)!=expected_value||boundary.events!=expected;differences+=bad;if(bad&&differences==1)std::cerr<<"first writer difference "<<ordinal<<'\n';
 }
 if(position!=bytes.size())return 2;
 std::cout<<"{\"cases\":2400,\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what();return 2;}
