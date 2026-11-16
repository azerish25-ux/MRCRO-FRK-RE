#include "mfrk/parameter_packet.hpp"
#include <array>
#include <bit>
using namespace mfrk::parameter_packet;
int main(){
 auto result=scale(0x7fc12345u,false,0,false,{},0);if(!result||!result->accepted||result->bits!=0x7fc12345u)return 1;
 result=scale(std::bit_cast<std::uint32_t>(.75f),true,4,true,{},0);if(!result||!result->accepted||result->bits!=0x3f800000u)return 2;
 const std::array<std::uint32_t,3> mapping{2,0,1};
 result=scale(std::bit_cast<std::uint32_t>(.5f),true,3,false,mapping,5);if(!result||!result->accepted||result->bits!=0)return 3;
 result=scale(0,true,1,true,{},0);if(!result||!result->accepted||result->bits!=0)return 4;
 result=scale(0x3f000000u,true,0,true,{},0);if(!result||result->accepted||result->bits!=0x3f000000u)return 5;
 result=scale(0x3f000000u,true,4,false,{},0);if(!result||result->accepted||result->bits!=0x3f000000u)return 6;
 if(scale(0x7fc00000u,true,4,true,{},0)||scale(0,true,4097,true,{},0))return 7;
 result=scale(std::bit_cast<std::uint32_t>(-.1f),true,4,true,{},0);if(!result||!result->accepted||result->bits!=0)return 8;
 result=scale(std::bit_cast<std::uint32_t>(1.1f),true,4,true,{},0);if(!result||!result->accepted||result->bits!=0x3f800000u)return 9;
}
