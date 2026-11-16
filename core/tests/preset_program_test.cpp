#include "mfrk/preset_program.hpp"
#include <array>
#include <bit>
using namespace mfrk::preset_program;
static std::array<std::uint8_t,4> bits(std::uint32_t v){return{std::uint8_t(v),std::uint8_t(v>>8),std::uint8_t(v>>16),std::uint8_t(v>>24)};}
static std::uint32_t value(const Write& w){std::uint32_t v=0;for(unsigned i=0;i<4;++i)v|=std::uint32_t(w.payload[i])<<(8*i);return v;}
int main(){
 const auto zero=bits(0),half=bits(0x3f000000u),quarter=bits(0x3e800000u);const std::array<std::uint8_t,3> data{4,5,6};
 const std::array<Descriptor,5> internal{{{0,0,1,zero,"Ignored"},{2,0,1,zero,"N"},{256,0x3e800000u,1,half,"R"},{514,0,3,data,"B"},{2,0,3,quarter,"F"}}};
 const std::array<Descriptor,2> vst{{{2,0,1,half,"V"},{0,0,1,zero,"Unselected"}}};
 const Numeric numeric{{"N",0x80000000u},{"R",0x3f800000u},{"V",0x3fc00000u},{"Unknown",42}};
 const Data saved{{"B",{}},{"F",{'a',0,'b'}},{"Unknown",{1,2}}};
 auto program=prepare_group0(internal,vst,numeric,saved);if(!program||program->size()!=5)return 1;
 const std::array<std::uint32_t,5> ids{0x80000001u,0x80000002u,0x80000003u,0x80000004u,0};for(unsigned i=0;i<5;++i)if((*program)[i].wrapper_id!=ids[i]||!(*program)[i].dirty)return 2;
 if(value((*program)[0])!=0x80000000u||value((*program)[1])!=0x3e800000u||(*program)[2].payload!=std::vector<std::uint8_t>{0,0,0}||(*program)[3].payload!=std::vector<std::uint8_t>{'a',0,'b',0}||value((*program)[4])!=0x3fc00000u)return 3;
 if((*program)[0].cached_normalized_double_bits||(*program)[4].cached_normalized_double_bits!=std::optional<std::uint64_t>{0x3ff0000000000000ull})return 4;
 if(numeric.at("Unknown")!=42||saved.at("Unknown")!=std::vector<std::uint8_t>{1,2})return 5;
 const std::array<Descriptor,1> bad{{{2,0,3,data,"Data"}}};if(prepare_group0({},bad,{},{}))return 6;
 auto empty=prepare_group0(internal,vst,{},{});if(!empty||empty->size()!=5||(*empty)[2].payload!=std::vector<std::uint8_t>{4,5,6})return 7;
}
