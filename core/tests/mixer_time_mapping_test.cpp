#include "mfrk/ui_slider_curves.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <fstream>
#include <iostream>
int main(int argc,char** argv){
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=4548)return 2;
 std::vector<std::uint8_t> bytes(4548);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"MIXTM001",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xfc,0xe3,0xf8,0x64,0x51,0x37,0x32,0x0e,0xde,0x3b,0xcf,0xd4,0xa1,0xcc,0x95,0x0d,0x56,0x26,0x83,0x4a,0xaa,0xce,0x70,0xdc,0xd1,0xd4,0x30,0x11,0x28,0x6e,0x8f,0xb7})return 2;
 auto word=[&](std::size_t p){std::uint32_t value;std::memcpy(&value,bytes.data()+p,4);return value;};if(word(8)!=378)return 2;
 auto curve=mfrk::ui::make_slider_curve(".?AVMixerTimeMapping@MappingTools@Arturia@@");if(!curve)return 2;
 unsigned differences=0;
 for(unsigned i=0;i<378;++i){const auto p=12+i*12;const auto direction=word(p);if(direction>1)return 2;const float value=std::bit_cast<float>(word(p+4));const auto result=direction?curve->inverse(value):curve->forward(value);if(std::bit_cast<std::uint32_t>(result)!=word(p+8)){if(!differences)std::cerr<<"first mixer-time mapping difference "<<i<<'\n';++differences;}}
 std::cout<<"{\"cases\":378,\"differences\":"<<differences<<"}\n";return differences?1:0;
}
