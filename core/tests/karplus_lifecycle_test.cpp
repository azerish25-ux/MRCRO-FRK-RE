#include "karplus_test_access.hpp"
#include <fstream>
#include <vector>
#include <iostream>
#include <stdexcept>
using namespace mfrk::karplus;
#include "karplus_fixture_reader.hpp"
using Reader=FixtureReader;
int main(int argc,char**argv)try{static_assert(std::endian::native==std::endian::little);if(argc!=2)return 2;Reader r(argv[1]);if(r.read<std::array<char,8>>()!=std::array<char,8>{'K','R','P','L','L','0','0','1'}||r.read<std::uint32_t>()!=2)return 2;Tables t;std::uint64_t commands=0,states=0,buffers=0,seeds=0,returns=0,audios=0;
for(unsigned side=0;side<2;++side){if(r.read<std::uint8_t>()!=side)return 2;SharedRandom rng{r.read<std::uint32_t>()};Memory m;for(unsigned i=0;i<8192;++i)m.samples[i]=s16(((i*7919u+12345u)&65535u)-32768u);auto expected=m.samples;auto e=Engine::admitted_zero_arena(t,rng,m);TestAccess::fill_constructor(e,165);const auto count=r.read<std::uint32_t>();if(!count||count>10000)return 2;
for(unsigned j=0;j<count;++j){const auto op=r.read<std::uint8_t>();const auto a=r.read<std::array<std::uint32_t,4>>();const auto ret=r.read<std::uint32_t>();const auto state=r.read<std::array<std::uint8_t,288>>();const auto seed=r.read<std::uint32_t>();std::array<std::uint32_t,32> audio{};if(op==14)audio=r.read<std::array<std::uint32_t,32>>();const auto n=r.read<std::uint16_t>();if(n>8192)return 2;int last=-1;for(unsigned k=0;k<n;++k){auto i=r.read<std::uint16_t>();if(i>=8192||i<=last)return 2;last=i;expected[i]=r.read<std::int16_t>();}std::uint32_t actualret=0;std::array<float,32> output{};
switch(op){case 1:e.bind(0x400020000ull,std::bit_cast<std::int32_t>(a[0]));break;case 2:e.initialize();break;case 3:e.set_pitch(F(a[0]),F(a[1]));break;case 4:e.trigger(a[0]!=0);break;case 5:e.set_phase(F(a[0]));break;case 6:e.reset();break;case 7:if(a[0]>2||a[1]>65535||!e.set_word(a[0],static_cast<std::uint16_t>(a[1])))return 2;break;case 8:if(a[0]>2||a[1]>255||!e.set_dirty(a[0],static_cast<std::uint8_t>(a[1])))return 2;break;case 9:if(a[0]>255)return 2;e.set_flag(static_cast<std::uint8_t>(a[0]));break;case 10:actualret=e.flag();break;case 11:case 12:{std::array<std::uint16_t,3>w{};for(unsigned k=0;k<3;++k){if(a[k]>65535)return 2;w[k]=static_cast<std::uint16_t>(a[k]);}if(op==11)e.fetch_words(w);else e.initialize_from_records(w);break;}case 13:e.prepare_current(a[0]!=0);break;case 14:if(a[0]>2||e.try_render(output,a[0]!=0)!=Status::ok)return 2;for(unsigned k=0;k<32;++k)audios+=std::bit_cast<std::uint32_t>(output[k])!=audio[k];break;case 15:e.set_level(F(a[0]));break;default:return 2;}
for(unsigned k=0;k<288;++k){states+=TestAccess::state(e)[k]!=state[k];}for(unsigned k=0;k<8192;++k){buffers+=m.samples[k]!=expected[k];}seeds+=rng.value!=seed;returns+=actualret!=ret;++commands;
}}
r.end();std::cout<<"commands "<<commands<<" states "<<states<<" buffers "<<buffers<<" seeds "<<seeds<<" returns "<<returns<<" audio "<<audios<<"\n";return states||buffers||seeds||returns||audios?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 2;}
