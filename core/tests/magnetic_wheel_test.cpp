#include "mfrk/ui.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <fstream>
#include <iostream>
using namespace mfrk::ui;
int main(int argc,char**argv)try{
 static_assert(std::endian::native==std::endian::little);if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=183524)return 2;
 std::vector<std::uint8_t> bytes(183524);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"MAGWHEE1",8))return 2;
 if(scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xa8,0x60,0x5f,0x6f,0x45,0x55,0xa7,0xf9,0xa7,0x77,0xd0,0x93,0x27,0x7a,0xf1,0x2a,0xbb,0xb8,0xa9,0xcf,0x96,0xbe,0x07,0xa5,0x10,0x7d,0x63,0xd5,0xf5,0x6f,0x52,0xfd})return 2;
 std::size_t position=8;auto read=[&]<class T>(){if(position>bytes.size()||sizeof(T)>bytes.size()-position)throw std::runtime_error("truncated");T result;std::memcpy(&result,bytes.data()+position,sizeof result);position+=sizeof result;return result;};
 if(read.operator()<std::uint32_t>()!=3744)return 2;
 unsigned differences=0,handled=0,writes=0;
 for(unsigned ordinal=0;ordinal<3744;++ordinal){
  const auto count=read.operator()<std::uint8_t>();if(count>128)return 2;std::vector<MagneticZone> zones;
  for(unsigned i=0;i<count;++i){const auto from=read.operator()<float>(),to=read.operator()<float>(),value_from=read.operator()<float>(),value_to=read.operator()<float>();const auto skip=read.operator()<std::uint8_t>();zones.push_back({from,to,value_from,value_to,bool(skip)});}
  const auto value=read.operator()<std::uint32_t>(),vertical=read.operator()<std::uint32_t>();const auto reversed=read.operator()<std::uint8_t>(),inertial=read.operator()<std::uint8_t>(),reverse=read.operator()<std::uint8_t>(),expected_handled=read.operator()<std::uint8_t>(),getters=read.operator()<std::uint8_t>(),targets=read.operator()<std::uint8_t>();if(reversed>1||inertial>1||reverse>1||expected_handled>1||targets>1||getters!=std::uint8_t(!inertial&&(vertical&0x7fffffffu)!=0))return 2;
  const auto target=targets?read.operator()<std::uint32_t>():0;const auto result=magnetic_wheel(zones,std::bit_cast<float>(value),std::bit_cast<float>(vertical),reversed,inertial,reverse);
  differences+=result.handled!=bool(expected_handled)||result.has_target!=bool(targets)||(targets&&std::bit_cast<std::uint32_t>(result.target)!=target);handled+=result.handled;writes+=result.has_target;
 }
 if(position!=bytes.size()||handled!=110||writes!=14)return 2;
 std::cout<<"{\"cases\":3744,\"handled\":"<<handled<<",\"writes\":"<<writes<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what();return 2;}
