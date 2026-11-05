#include "karplus_test_access.hpp"
#include <cstdlib>
#include <new>
#include <iostream>
static std::size_t allocations=0;static bool observe=false;
void* operator new(std::size_t n){if(observe)++allocations;if(void*p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void*p)noexcept{std::free(p);}void operator delete(void*p,std::size_t)noexcept{std::free(p);}
void* operator new[](std::size_t n){return ::operator new(n);}void operator delete[](void*p)noexcept{::operator delete(p);}void operator delete[](void*p,std::size_t)noexcept{::operator delete(p);}
int main(){using namespace mfrk::karplus;Tables t;SharedRandom rng{54321};Memory m{};auto e=Engine::admitted_zero_arena(t,rng,m);e.bind(0x400020000,8192);e.initialize();Input in{};in.frequency_hz=440;in.note=69;in.level=F(0x40086f69);in.words={32768,40000,50000};in.dirty={1,1,1};std::array<float,32>out;observe=true;for(unsigned i=0;i<1024;++i){in.trigger_event=i%128==0?2:0;if(e.try_process(in,out)!=Status::ok)return 1;}TestAccess::write(e,0x90,std::uint32_t{0x80001000});in.trigger_event=0;for(unsigned i=0;i<1024;++i){if(e.try_process(in,out)!=Status::ok)return 1;}observe=false;std::cout<<"rendered_blocks 2048 dynamic_allocations "<<allocations<<"\n";return allocations?1:0;}
