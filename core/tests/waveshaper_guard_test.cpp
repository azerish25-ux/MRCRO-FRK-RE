#include <cmath>
#include "mfrk/waveshaper.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <iostream>
#include <limits>
using namespace mfrk::waveshaper;
namespace mfrk::waveshaper {
inline float F(std::uint32_t x) noexcept {return std::bit_cast<float>(x);}
struct TestAccess {
 static float frequency(const Tables&t,float note) noexcept {return t.frequency(note);}
 static std::int32_t conversion(float x) noexcept {return Engine::cvtt32(x);}
 static void store(Engine& e,unsigned offset,std::uint32_t bits) {std::memcpy(e.state_.data()+offset,&bits,4);}
 static void trace_sentinels(Engine& e) {e.slope_.fill(F(0x3eabcdef));e.folded_.fill(F(0xbeabcdef));}
 struct Snapshot {std::array<std::uint8_t,216> state;std::array<float,16> slope,fold;bool initialized;};
 static Snapshot snapshot(const Engine& e) {return {e.state_,e.slope_,e.folded_,e.initialized_};}
 static bool same(const Engine& e,const Snapshot& s) {return e.state_==s.state&&e.initialized_==s.initialized&&std::memcmp(e.slope_.data(),s.slope.data(),sizeof s.slope)==0&&std::memcmp(e.folded_.data(),s.fold.data(),sizeof s.fold)==0;}
};
}
int main(){const Tables tables;unsigned checks=0;
 const auto failure=[&](Engine& e,RenderStatus wanted,int api,bool init=false) {
  TestAccess::trace_sentinels(e);const auto snapshot=TestAccess::snapshot(e);
  std::array<float,32> output;output.fill(F(0x3e123456));const auto original=output;
  RenderStatus status;
  if(api==0)status=e.try_render_raw(output);
  else if(api==1)status=e.try_render_scaled(output);
  else {Input in{};in.pitch_argument=69;in.words={111,222,333};in.dirty={0,0,0};in.gate=true;in.initialize=init;in.output_level=.25f;status=e.try_process(in,output);}
  ++checks;return status==wanted&&TestAccess::same(e,snapshot)&&std::memcmp(output.data(),original.data(),sizeof output)==0;
 };
 for(int api=0;api<3;++api){auto e=Engine::admitted_zero_arena(tables);if(!failure(e,RenderStatus::not_initialized,api))return 1;}
 for(auto bits:{0x40000000u,0xc0000000u,0x7f800000u,0x7fc00000u})for(int api=0;api<3;++api){auto e=Engine::admitted_zero_arena(tables);e.initialize();TestAccess::store(e,0x6c,bits);if(!failure(e,RenderStatus::shape_domain,api))return 2;}
 for(auto bits:{0x7149f2cau,0x7f800000u,0x7fc00000u})for(int api=0;api<3;++api){auto e=Engine::admitted_zero_arena(tables);e.initialize();TestAccess::store(e,0x70,bits);if(!failure(e,RenderStatus::fold_domain,api))return 3;}
 for(auto offset:{0x88u,0x90u}){auto e=Engine::admitted_zero_arena(tables);TestAccess::store(e,offset,0x7fc00000u);if(!failure(e,offset==0x88?RenderStatus::fold_domain:RenderStatus::shape_domain,2,true))return 4;}
 for(auto bits:{0xff800000u,0x7f800000u,0x7fc00000u,0xff7fffffu,0x7f7fffffu,0xcf000000u,0x4f000000u}){const float frequency=TestAccess::frequency(tables,F(bits));if(!std::isfinite(frequency)||frequency<=0)return 5;++checks;}
 if(TestAccess::conversion(F(0x7fc00000))!=INT32_MIN||TestAccess::conversion(F(0x7f800000))!=INT32_MIN||TestAccess::conversion(F(0x4f000000))!=INT32_MIN||TestAccess::conversion(F(0x4effffff))!=2147483520||TestAccess::conversion(F(0xcf000000))!=INT32_MIN||TestAccess::conversion(F(0xcf000001))!=INT32_MIN)return 6;
 checks+=6;std::cout<<"{\"release_guard_checks\":"<<checks<<",\"mismatches\":0,\"scope\":\"Test-only injected out-of-domain histories/held parameters exercise release statuses and full transaction; not native sound/equivalence expectations.\"}\n";return 0;
}
