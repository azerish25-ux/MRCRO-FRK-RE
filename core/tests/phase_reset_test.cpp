#include "mfrk/phase_reset.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include "adsr_test_access.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace mfrk::phase_reset;
struct Reader{std::vector<std::uint8_t>b;std::size_t p=0;template<class T>T get(){if(p+sizeof(T)>b.size())throw std::runtime_error("truncated");T x;std::memcpy(&x,b.data()+p,sizeof x);p+=sizeof x;return x;}};
template<class T>T field(const std::array<std::uint8_t,160>& b,unsigned offset){T x;std::memcpy(&x,b.data()+offset,sizeof x);return x;}
int main(int argc,char**argv)try{
 if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);Reader r{{std::istreambuf_iterator<char>(f),{}}};
 if(r.b.size()!=61644||mfrk::ui::scene::scope::test::sha256(r.b)!=std::array<std::uint8_t,32>{0xc9,0xc2,0xc8,0xd3,0x91,0x80,0x82,0xa8,0x49,0x9f,0x9c,0x3a,0xb7,0x12,0x57,0xb7,0x04,0x75,0x69,0x49,0x10,0xee,0xfd,0xa1,0x71,0xbd,0x56,0xd4,0xbf,0x4f,0xdb,0x6c})throw std::runtime_error("fixture identity");
 for(char c:std::array<char,8>{'P','H','A','S','E','R','S','1'})if(r.get<char>()!=c)return 2;auto count=r.get<std::uint32_t>();if(count!=192)return 2;unsigned differences=0;
 for(unsigned i=0;i<count;++i){const auto kind=r.get<std::uint8_t>();if(kind>1)return 2;auto before=r.get<std::array<std::uint8_t,160>>();const auto after=r.get<std::array<std::uint8_t,160>>();const unsigned shift=kind?16:0;Phase state{field<float>(before,0x54+shift),field<float>(before,0x60+shift),field<float>(before,0x40+shift),field<float>(before,0x44+shift),field<float>(before,0x48+shift)};reset(state);std::memcpy(before.data()+0x44+shift,&state.previous,4);std::memcpy(before.data()+0x48+shift,&state.current,4);differences+=before!=after;}
 if(r.p!=r.b.size())return 2;
 mfrk::Adsr env;mfrk::AdsrState initial{};initial.phase=mfrk::AdsrPhase::idle;mfrk::AdsrTestAccess::load_explicit_fixture(env,initial);
 Cyclic cyclic;cyclic.envelope=&env;cyclic.phase={.2f,.5f,.75f,7.f,8.f};cyclic.flag91=9;cyclic.flag92=10;cyclic.flag93=127;cyclic.flag94=12;cyclic.flag95=13;
 if(!gate(cyclic,true)||std::bit_cast<std::uint32_t>(cyclic.phase.previous)!=0x3e4ccccdu||std::bit_cast<std::uint32_t>(cyclic.phase.current)!=0x3f533333u||cyclic.flag91!=0||cyclic.flag92!=1||cyclic.flag93!=127||cyclic.flag94!=12||cyclic.flag95!=127)return 2;
 initial.phase=mfrk::AdsrPhase::sustain;initial.current=123;initial.accumulator=0x40000000;initial.hold=1;initial.pending_release=1;mfrk::AdsrTestAccess::load_explicit_fixture(env,initial);reset_cyclic(env,cyclic.phase);auto after=env.snapshot();Shared shared{0xdeadbeefu,0x12345678u};reset_shared(shared);
 if(after.phase!=mfrk::AdsrPhase::idle||after.current!=0||after.accumulator!=0||after.hold||after.pending_release||std::bit_cast<std::uint32_t>(cyclic.phase.current)!=0x3f533333u||shared.field18!=0||shared.field60!=3)return 2;
 std::cout<<count<<" original phase resets, "<<differences<<" differences\n";return differences?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
