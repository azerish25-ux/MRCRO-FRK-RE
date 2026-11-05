#include "voice_audio_test_access.hpp"
#include <cstdlib>
#include <new>
#include <iostream>
static bool measuring=false;static unsigned allocations=0;
void* operator new(std::size_t n){if(measuring)++allocations;if(void*p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void*p)noexcept{std::free(p);}void operator delete(void*p,std::size_t)noexcept{std::free(p);}
using namespace mfrk::voice_audio;

void render(void*,unsigned side,const void*,std::span<float,32>output)noexcept{for(unsigned i=0;i<32;++i)output[i]=(static_cast<float>(i)-16.f)*(side?.025f:.03125f);}
int main(){auto v=VoiceAudio::admitted_zero_owner_arena();Controls c{65532,32766,0,1,1,false,true,0,{32766,65532,65532,0},{1,1,1,1}};if(!v.prepare(c))return 1;const auto before=TestAccess::modes(v);const auto numeric_before=TestAccess::snapshot(v);c.selected_lane=2;c.para=true;c.cutoff_word=123;if(v.prepare(c)||before!=TestAccess::modes(v)||numeric_before!=TestAccess::snapshot(v))return 1;std::array<float,32>a{},b{};a.fill(3.f);b.fill(-4.f);const auto saved_a=a,saved_b=b;if(v.render({nullptr,nullptr},nullptr,a,b)||a!=saved_a||b!=saved_b||numeric_before!=TestAccess::snapshot(v)||before!=TestAccess::modes(v))return 1;c.selected_lane=0;c.para=false;measuring=true;for(unsigned n=0;n<2048;++n){c.volume_word=static_cast<std::uint16_t>(n%65533);if(!v.prepare(c)||!v.render({nullptr,render},nullptr,a,b))return 1;}measuring=false;std::cout<<"2048blocks allocations "<<allocations<<"; invalidlane/nullcallback guards pass\n";return allocations?1:0;}
