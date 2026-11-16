#include "mfrk/ui_slider_drag.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <fstream>
#include <iostream>
#include <vector>
using namespace mfrk::ui;
struct Identity:SliderCurve {float forward(float x)override{return x;}float inverse(float x)override{return x;}};
struct Boundary:SliderBeginBoundary {
 SliderValueInfo& value_info;std::array<std::uint32_t,31>& row;std::vector<unsigned> calls;
 Boundary(SliderValueInfo& i,std::array<std::uint32_t,31>& r):value_info(i),row(r){}
 void prepare(std::uint32_t id)override{if(id!=row[9])throw 1;calls.push_back(0);}
 SliderValueInfo& info(std::uint32_t id,bool custom)override{if(id!=row[9]||custom!=bool(row[6]))throw 1;if(!custom)calls.push_back(1);return value_info;}
 void edit(std::uint32_t id,std::uint32_t modifiers,bool secondary)override{if(id!=row[9]||modifiers!=((row[15]>>5)&0xffffff01u)||secondary)throw 1;calls.push_back(2);}
 bool blocked(std::uint32_t id)override{if(id!=row[9])throw 1;calls.push_back(3);return (row[16]&&row[17])||(row[18]&&(row[19]||row[20]))||(row[21]&&row[22]);}
 float sensitivity(const SliderDragEvent&,std::uint32_t id)override{if(id!=row[9])throw 1;calls.push_back(4);return std::bit_cast<float>(row[10]);}
 std::array<std::int32_t,2> global_position(const SliderDragEvent&)override{calls.push_back(5);return {std::bit_cast<std::int32_t>(row[13]),std::bit_cast<std::int32_t>(row[14])};}
};
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 {
  std::ifstream fixture(argv[1],std::ios::binary|std::ios::ate);if(!fixture||fixture.tellg()!=31756)return 2;
  std::vector<std::uint8_t> bytes(31756);fixture.seekg(0);if(!fixture.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))return 2;
  if(scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xa9,0x37,0xd8,0xef,0x27,0x77,0x2e,0x98,0x15,0x6f,0xa5,0x4b,0x35,0x16,0x2c,0xb6,0x78,0xe8,0x70,0x6a,0xfe,0x08,0x85,0xe5,0x29,0x46,0xf1,0x61,0x67,0x0b,0x6f,0x68})return 2;
 }
 std::ifstream input(argv[1],std::ios::binary);char magic[8]{};input.read(magic,8);if(std::string_view(magic,8)!="SLBEG001")return 2;
 std::uint32_t count=0;input.read(reinterpret_cast<char*>(&count),4);if(count!=256)return 2;unsigned differences=0;Identity identity;
 for(unsigned ordinal=0;ordinal<count;++ordinal){
  std::array<std::uint32_t,31> r{};if(!input.read(reinterpret_cast<char*>(r.data()),sizeof r))return 2;
  auto f=[&](unsigned i){return std::bit_cast<float>(r[i]);};auto n=[&](unsigned i){return std::bit_cast<std::int32_t>(r[i]);};
  SliderValueInfo info{identity,f(0),f(1),n(2),n(3),bool(r[4]),bool(r[5])};Boundary boundary(info,r);
  SliderDragState state{.375f,17,.02f,-15.f,22.f,true,true};SliderDragConfig config{n(7),.001f,false,false,bool(r[6])};SliderValueConfig value_config{false,0,99.f,false,false,false};
  slider_begin(state,config,value_config,{n(11),n(12)},r[15],r[9],f(8),boundary);
  const std::array<std::uint32_t,8> actual{std::bit_cast<std::uint32_t>(state.value_anchor),state.parameter_id,std::bit_cast<std::uint32_t>(state.sensitivity),std::bit_cast<std::uint32_t>(state.global_anchor),std::bit_cast<std::uint32_t>(state.local_anchor),std::uint32_t(state.global_tracking),std::uint32_t(state.outside),std::bit_cast<std::uint32_t>(value_config.offset)};
  std::vector<unsigned> expected_calls{0};if(!r[6])expected_calls.push_back(1);expected_calls.push_back(2);expected_calls.push_back(3);
  const bool blocked=(r[16]&&r[17])||(r[18]&&(r[19]||r[20]))||(r[21]&&r[22]);if(!blocked){expected_calls.push_back(4);expected_calls.push_back(5);}
  bool bad=boundary.calls!=expected_calls;for(unsigned i=0;i<8;++i)bad|=actual[i]!=r[23+i];if(bad&&!differences)std::cerr<<"first begin difference "<<ordinal<<'\n';differences+=bad;
 }
 if(input.peek()!=std::char_traits<char>::eof())return 2;
 std::cout<<"{\"cases\":256,\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(...){return 2;}
