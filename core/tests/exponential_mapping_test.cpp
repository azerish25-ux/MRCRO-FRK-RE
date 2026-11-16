#include "mfrk/ui_slider_curves.hpp"
#include "../src/ui_exponential_math.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
int main(int argc,char** argv){
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=333996)return 2;
 std::vector<std::uint8_t> bytes(333996);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"EXPREF01",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xc0,0x7c,0x48,0xd2,0x55,0x31,0x53,0x83,0xc2,0x60,0xc1,0x83,0xda,0x2b,0x64,0xb4,0xa4,0xe9,0xf8,0x09,0xbb,0xe5,0xd8,0xb7,0x33,0x57,0x2e,0x2a,0xb0,0x0d,0xbe,0x65})return 2;
 auto word=[&](std::size_t p){std::uint32_t value;std::memcpy(&value,bytes.data()+p,4);return value;};if(word(8)!=20874)return 2;
 constexpr auto name=".?AVExponentialPotardMapping@MappingTools@Arturia@@";
 constexpr std::array<std::array<std::uint32_t,2>,3> fields{{{0x40c00000,0x3b22d9e6},{0x40a00000,0x3bde496c},{0x411c0000,0x38748512}}};
 std::array<std::unique_ptr<mfrk::ui::SliderCurve>,3> curves;
 for(unsigned i=0;i<3;++i){curves[i]=mfrk::ui::make_slider_curve(name,{std::bit_cast<float>(fields[i][0]),std::bit_cast<float>(fields[i][1]),0,0});if(!curves[i])return 2;}
 if(mfrk::ui::make_slider_curve(name)||mfrk::ui::make_slider_curve(name,{88,1,0,0})||mfrk::ui::make_slider_curve(name,{6,0,0,0}))return 2;
 unsigned differences=0;
 for(unsigned i=0;i<20874;++i){
  const std::size_t p=12+std::size_t(i)*16;const auto mode=word(p),profile=word(p+4);if(mode>3||profile>2||(mode>=2&&profile))return 2;
  const float value=std::bit_cast<float>(word(p+8));
  const float result=mode==0?curves[profile]->forward(value):mode==1?curves[profile]->inverse(value):mode==2?mfrk::ui::detail::exponential_exp(value):mfrk::ui::detail::exponential_log(value);
  if(std::bit_cast<std::uint32_t>(result)!=word(p+12)){if(differences++<4)std::cerr<<"Exponential mapping difference "<<i<<'\n';}
 }
 std::cout<<"{\"cases\":20874,\"differences\":"<<differences<<"}\n";return differences?1:0;
}
