#include "mfrk/ui_slider_curves.hpp"
#include "../src/ui_master_power.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <fstream>
#include <iostream>
int main(int argc,char** argv){
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=178044)return 2;
 std::vector<std::uint8_t> bytes(178044);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"MSTVOL01",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x1b,0xc2,0xfa,0x69,0x4f,0x9b,0x77,0xbd,0x86,0x0b,0x3c,0x87,0xe5,0x66,0x7c,0x88,0x72,0x62,0xdb,0xff,0x0b,0xee,0x12,0x3b,0x92,0x3e,0x6d,0x80,0xfb,0xa6,0x3c,0xdb})return 2;
 auto word=[&](std::size_t p){std::uint32_t value;std::memcpy(&value,bytes.data()+p,4);return value;};if(word(8)!=14836)return 2;
 constexpr auto name=".?AVBipolarVariableCenterInvPowPotardMapping@MappingTools@Arturia@@";
 auto curve=mfrk::ui::make_slider_curve(name,{2.f,0,.5f,0});if(!curve)return 2;
 if(mfrk::ui::make_slider_curve(name,{3.f,0,.5f,0})||mfrk::ui::make_slider_curve(name,{2.f,0,.4f,0})||mfrk::ui::make_slider_curve(name))return 2;
 unsigned differences=0;
 for(unsigned i=0;i<14836;++i){
  const auto p=12+i*12,direction=word(p);if(direction>3)return 2;
  const float value=std::bit_cast<float>(word(p+4));
  const float result=direction==0?curve->forward(value):direction==1?curve->inverse(value):mfrk::ui::detail::master_power(value,direction==3);
  if(std::bit_cast<std::uint32_t>(result)!=word(p+8)){if(differences++<4)std::cerr<<"Master-volume mapping difference "<<i<<'\n';}
 }
 std::cout<<"{\"cases\":14836,\"differences\":"<<differences<<"}\n";return differences?1:0;
}
