#include "harmo_test_access.hpp"
#include "karplus_test_access.hpp"
#include "karplus_fixture_reader.hpp"
#include <algorithm>
#include <bit>
#include <iostream>
#include <stdexcept>
namespace H=mfrk::harmo;
namespace K=mfrk::karplus;
struct Snapshot {
 std::array<std::uint8_t,288> k;
 std::array<std::uint8_t,424> h;
 std::uint32_t rng;
 std::uint8_t active,hactive;
};
Snapshot snapshot(FixtureReader&r) {
 const Snapshot value{r.read<std::array<std::uint8_t,288>>(),r.read<std::array<std::uint8_t,424>>(),r.read<std::uint32_t>(),r.read<std::uint8_t>(),r.read<std::uint8_t>()};
 if((value.active!=0&&value.active!=2&&value.active!=3)||value.hactive>1)throw std::runtime_error("owner flags");
 return value;
}
int main(int argc,char**argv)try {
 if(argc!=2)return 2;
 FixtureReader r(argv[1]);
 if(r.read<std::array<char,8>>()!=std::array<char,8>{'S','L','O','T','A','L','0','1'})return 2;
 const auto count=r.read<std::uint32_t>();if(count!=206)return 2;
 auto expected=snapshot(r);
 auto expected_memory=r.read<std::array<std::int16_t,8192>>();
 if(expected.rng!=54321||std::any_of(expected_memory.begin(),expected_memory.end(),[](auto x){return x!=0;}))return 2;
 K::Tables kt;H::Tables ht;K::Memory memory{};K::SharedRandom rng{54321};
 auto k=K::Engine::admitted_zero_arena(kt,rng,memory);
 auto h=H::Engine::admitted_zero_arena_uninitialized(ht,std::span<std::int16_t>(memory.samples).first<2048>());
 std::uint8_t active=0;bool hactive=false;
 std::uint64_t state_errors=0,memory_errors=0,random_errors=0,owner_errors=0,audio_errors=0,raw_errors=0,blocks=0;
 const auto check=[&](const Snapshot&e,unsigned op) {
  auto ks=K::TestAccess::state(k);auto hs=H::TestAccess::state(h);
  std::fill(ks.begin()+0x18,ks.begin()+0x20,0);std::fill(hs.begin()+0x18,hs.begin()+0x20,0);
  for(unsigned j=0;j<ks.size();++j)if(ks[j]!=e.k[j]) {if(state_errors<12)std::cerr<<"op "<<op<<" K offset "<<std::hex<<j<<" got "<<unsigned(ks[j])<<" expected "<<unsigned(e.k[j])<<std::dec<<'\n';++state_errors;}
  for(unsigned j=0;j<hs.size();++j)if(hs[j]!=e.h[j]) {if(state_errors<12)std::cerr<<"op "<<op<<" H offset "<<std::hex<<j<<" got "<<unsigned(hs[j])<<" expected "<<unsigned(e.h[j])<<std::dec<<'\n';++state_errors;}
  for(unsigned j=0;j<memory.samples.size();++j)memory_errors+=memory.samples[j]!=expected_memory[j];
  random_errors+=rng.value!=e.rng;owner_errors+=active!=e.active;owner_errors+=hactive!=(e.hactive!=0);
 };
 check(expected,0);
 for(unsigned i=0;i<count;++i) {
  const auto op=r.read<std::uint8_t>(),model=r.read<std::uint8_t>();
  const auto a=r.read<std::uint32_t>(),b=r.read<std::uint32_t>();
  const auto x=r.read<std::uint32_t>(),y=r.read<std::uint32_t>();
  expected=snapshot(r);
  const auto changes=r.read<std::uint16_t>();if(changes>8192)return 2;int last=-1;
  for(unsigned j=0;j<changes;++j){const auto index=r.read<std::uint16_t>();if(index>=8192||index<=last)return 2;last=index;expected_memory[index]=r.read<std::int16_t>();}
  if(model!=2&&model!=3)return 2;
  if(op!=1&&model!=active)return 2;
  const bool is_k=model==3;
  std::array<float,32> output{};
  switch(op) {
   case 1:
    if(a!=0||b!=model)return 2;
    if(active!=model) {
     if(is_k)k.set_flag(1);else hactive=true;
     if(active==3){k.reset();k.set_flag(0);}else if(active==2){h.reset();hactive=false;}
     if(is_k){k.bind(0x400020000ull,16384);k.set_phase(0);k.initialize();}
     else{h.bind_buffer(true,16384,0x400020000ull);h.set_phase(0);h.initialize();}
     active=model;
    }
    break;
   case 2:if(is_k)k.set_pitch(std::bit_cast<float>(x),std::bit_cast<float>(y));else h.set_frequency(std::bit_cast<float>(x));break;
   case 3:if(a>2||b>65535)return 2;if(is_k){if(!k.set_word(a,static_cast<std::uint16_t>(b)))return 2;}else if(!h.set_word(a,static_cast<std::uint16_t>(b)))return 2;break;
   case 4:if(a>2||b>255)return 2;if(is_k){if(!k.set_dirty(a,static_cast<std::uint8_t>(b)))return 2;}else if(!h.set_dirty(a,static_cast<std::uint8_t>(b)))return 2;break;
   case 5:if(a>1)return 2;if(is_k)k.trigger(a!=0);else h.trigger(a!=0);break;
   case 6:if(is_k)k.prepare_current(true);else h.prepare_current();break;
   case 7:if(is_k){if(k.try_render(output)!=K::Status::ok)throw std::runtime_error("K render");}else output=h.render_scaled();break;
   default:return 2;
  }
  if(op==7) {
   const auto raw=r.read<std::array<std::int16_t,32>>();
   const auto scaled=r.read<std::array<std::uint32_t,32>>();
   for(unsigned j=0;j<32;++j) {
    std::int16_t actual;
    if(is_k)actual=K::TestAccess::raw(k)[j];else std::memcpy(&actual,H::TestAccess::state(h).data()+0x3c+2*j,2);
    raw_errors+=actual!=raw[j];audio_errors+=std::bit_cast<std::uint32_t>(output[j])!=scaled[j];
   }
   ++blocks;
  }
  check(expected,i+1);
 }
 r.end();
 const auto before_guards=H::TestAccess::state(h);
 if(h.set_word(3,1)||h.set_dirty(3,1)||before_guards!=H::TestAccess::state(h))return 1;
 std::cout<<"ops "<<count<<" blocks "<<blocks<<" state "<<state_errors<<" memory "<<memory_errors<<" rng "<<random_errors<<" owner "<<owner_errors<<" raw "<<raw_errors<<" audio "<<audio_errors<<'\n';
 return state_errors||memory_errors||random_errors||owner_errors||raw_errors||audio_errors?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
