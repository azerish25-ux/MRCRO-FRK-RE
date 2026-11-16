#include "mfrk/preset_migration.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace mfrk::preset;
static_assert(std::endian::native==std::endian::little);
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=88332)return 2;
 std::vector<char> data(88332);file.seekg(0);if(!file.read(data.data(),data.size())||std::memcmp(data.data(),"ONAP0001",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(std::span(reinterpret_cast<const std::uint8_t*>(data.data()),data.size()))!=std::array<std::uint8_t,32>{0x78,0xdd,0xd6,0x19,0xd9,0x2c,0x1f,0xbe,0x35,0x22,0x8d,0x81,0xf8,0xbd,0xd2,0xca,0x5a,0x81,0xb3,0x6f,0x9c,0xe2,0xfd,0x2b,0x01,0x8a,0x24,0x36,0x20,0x97,0xfb,0x6d})return 2;
 std::size_t offset=8;
 auto read=[&]<class T>(){if(offset>data.size()||sizeof(T)>data.size()-offset)throw std::runtime_error("truncated");T v;std::memcpy(&v,data.data()+offset,sizeof v);offset+=sizeof v;return v;};
 if(read.operator()<std::uint32_t>()!=3840)return 2;
 unsigned differences=0;
 for(unsigned i=0;i<3840;++i){
  const auto flags=read.operator()<std::uint8_t>();if(flags>63)return 2;
  const auto saved=read.operator()<std::uint32_t>(),fallback=read.operator()<std::uint32_t>(),reset=read.operator()<std::uint32_t>(),current=read.operator()<std::uint32_t>(),expected=read.operator()<std::uint32_t>();const auto dirty=read.operator()<std::uint8_t>(),notifies=read.operator()<std::uint8_t>();if(dirty>1||notifies>1)return 2;
  const auto result=apply_numeric(NumericApplyInput{saved,fallback,reset,current,static_cast<std::uint32_t>((flags>>3)&1),(flags&1)!=0,(flags&2)!=0,(flags&4)!=0,(flags&32)!=0});
  bool bad=result.bits!=expected||result.dirty!=static_cast<bool>(dirty)||result.notification.has_value()!=static_cast<bool>(notifies);
  if(result.notification){const auto& n=*result.notification;bad|=n.origin!=4||n.tag!=1||n.length!=4||n.bits!=expected;}
  differences+=bad;
 }
 if(offset!=data.size())return 2;
 std::cout<<"{\"cases\":3840,\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& e){std::cerr<<e.what();return 2;}
