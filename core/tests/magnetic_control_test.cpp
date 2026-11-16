#include "mfrk/ui.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <fstream>
#include <iostream>
using namespace mfrk::ui;
int main(int argc,char**argv)try{
 static_assert(std::endian::native==std::endian::little);if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=491272)return 2;
 std::vector<std::uint8_t> bytes(491272);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"MAGCTRL1",8))return 2;
 if(scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x8d,0x8c,0xf7,0x7e,0xd8,0x1f,0x9f,0xc6,0x1c,0x80,0xe1,0xc3,0x81,0x05,0xc5,0x3e,0x78,0x33,0x0f,0x50,0x76,0x0e,0x6c,0xb2,0x87,0xcd,0x52,0x32,0xd6,0xc2,0xf1,0x81})return 2;
 std::size_t position=8;auto read=[&]<class T>(){if(position>bytes.size()||sizeof(T)>bytes.size()-position)throw std::runtime_error("truncated");T value;std::memcpy(&value,bytes.data()+position,sizeof value);position+=sizeof value;return value;};
 if(read.operator()<std::uint32_t>()!=5040||read.operator()<std::uint32_t>()!=6674)return 2;
 unsigned phase_differences=0,delta_differences=0;
 for(unsigned i=0;i<5040;++i){
  const auto phase=read.operator()<std::uint32_t>(),value=read.operator()<std::uint32_t>(),held=read.operator()<std::uint32_t>();const auto flag20=read.operator()<std::uint8_t>(),flag21=read.operator()<std::uint8_t>(),matches=read.operator()<std::uint8_t>(),reverse=read.operator()<std::uint8_t>();const auto after_held=read.operator()<std::uint32_t>();const auto after20=read.operator()<std::uint8_t>(),after21=read.operator()<std::uint8_t>(),getters=read.operator()<std::uint8_t>();if(flag20>1||flag21>1||matches>1||reverse>1||after20>1||after21>1||getters!=matches)return 2;
  MagneticGesture state{std::bit_cast<float>(held),bool(flag20),bool(flag21)};magnetic_phase(state,phase,matches,std::bit_cast<float>(value),reverse);phase_differences+=std::bit_cast<std::uint32_t>(state.held_value)!=after_held||state.flag20!=bool(after20)||state.flag21!=bool(after21);
 }
 for(unsigned i=0;i<6674;++i){
  const auto count=read.operator()<std::uint8_t>();if(count>128)return 2;std::vector<MagneticZone> zones;
  for(unsigned j=0;j<count;++j){const auto from=read.operator()<float>(),to=read.operator()<float>(),value_from=read.operator()<float>(),value_to=read.operator()<float>();const auto skip=read.operator()<std::uint8_t>();if(skip>1)return 2;zones.push_back({from,to,value_from,value_to,bool(skip)});}
  const auto value=read.operator()<std::uint32_t>(),delta=read.operator()<std::uint32_t>();const auto flag20=read.operator()<std::uint8_t>(),flag21=read.operator()<std::uint8_t>(),wrap=read.operator()<std::uint8_t>(),matches=read.operator()<std::uint8_t>();const auto expected=read.operator()<std::uint32_t>();const auto skip=read.operator()<std::uint8_t>();if(flag20>1||flag21>1||wrap>1||matches>1||skip>1)return 2;
  auto result=magnetic_delta(zones,std::bit_cast<float>(value),std::bit_cast<float>(delta),flag20,flag21,wrap,matches);delta_differences+=std::bit_cast<std::uint32_t>(result.value)!=expected||result.skip!=bool(skip);
 }
 if(position!=bytes.size())return 2;
 std::cout<<"{\"phase_cases\":5040,\"delta_cases\":6674,\"phase_differences\":"<<phase_differences<<",\"delta_differences\":"<<delta_differences<<"}\n";return phase_differences||delta_differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what();return 2;}
