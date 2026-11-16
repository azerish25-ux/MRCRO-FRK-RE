#include "mfrk/ui_slider_curves.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <fstream>
#include <iostream>
int main(int argc,char** argv){
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=9108)return 2;
 std::vector<std::uint8_t> bytes(9108);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"ENVMP001",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xb8,0xe3,0xbd,0x3e,0xca,0xee,0x36,0x06,0xfb,0xef,0x4e,0xad,0x20,0xf5,0x85,0x3f,0xf4,0x26,0xe5,0x35,0xfe,0x71,0x90,0x06,0x70,0x2b,0x9a,0xaa,0x17,0x3a,0x3b,0x63})return 2;
 auto word=[&](std::size_t p){std::uint32_t value;std::memcpy(&value,bytes.data()+p,4);return value;};if(word(8)!=758)return 2;
 auto curve=mfrk::ui::make_slider_curve(".?AVEnveloppeTimeMapping@MappingTools@Arturia@@");if(!curve)return 2;
 unsigned differences=0;
 for(unsigned i=0;i<758;++i){const auto p=12+i*12;const auto direction=word(p);if(direction>1)return 2;const float value=std::bit_cast<float>(word(p+4));const auto result=direction?curve->inverse(value):curve->forward(value);if(std::bit_cast<std::uint32_t>(result)!=word(p+8)){if(!differences)std::cerr<<"first envelope mapping difference "<<i<<'\n';++differences;}}
 std::cout<<"{\"cases\":758,\"differences\":"<<differences<<"}\n";return differences?1:0;
}
