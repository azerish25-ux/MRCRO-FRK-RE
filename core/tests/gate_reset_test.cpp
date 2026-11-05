#include "mfrk/gate_dispatch.hpp"
#include "adsr_test_access.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace mfrk::gate_dispatch;
struct Reader{std::vector<std::uint8_t>b;std::size_t p=0;template<class T>T get(){if(p+sizeof(T)>b.size())throw std::runtime_error("truncated");T x;std::memcpy(&x,b.data()+p,sizeof x);p+=sizeof x;return x;}};
int main(int argc,char**argv)try{
 if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);Reader r{{std::istreambuf_iterator<char>(f),{}}};
 if(r.b.size()!=606||mfrk::ui::scene::scope::test::sha256(r.b)!=std::array<std::uint8_t,32>{0x27,0x66,0x26,0x66,0x6e,0x09,0xa0,0xc9,0xdb,0xf2,0x94,0x0b,0xa3,0xed,0x65,0x3f,0xde,0x81,0x68,0xba,0x5a,0xbe,0x45,0x66,0x05,0x83,0xd7,0x23,0x57,0x30,0xf8,0x1e})throw std::runtime_error("fixture identity");
 for(char c:std::array<char,8>{'G','A','T','E','R','S','T','1'})if(r.get<char>()!=c)return 2;auto count=r.get<std::uint32_t>();if(count!=18)return 2;unsigned differences=0;
 for(unsigned i=0;i<count;++i){const auto row=r.get<std::uint8_t>(),active=r.get<std::uint8_t>(),n=r.get<std::uint8_t>();std::vector<Action> expected;for(unsigned j=0;j<n;++j)expected.push_back({static_cast<Kind>(r.get<std::uint8_t>()),r.get<std::uint8_t>(),r.get<std::uint8_t>(),r.get<std::uint8_t>()!=0});const auto after=r.get<std::uint8_t>(),phase=r.get<std::uint8_t>();const auto current=r.get<std::int16_t>();const auto accumulator=r.get<std::int32_t>();const auto hold=r.get<std::uint8_t>(),pending=r.get<std::uint8_t>();
  State state;state.active_count=active;Plan plan;if(!dispatch(state,row,5,false,0,false,plan))return 2;mfrk::Adsr env;mfrk::AdsrState initial{};initial.phase=mfrk::AdsrPhase::sustain;initial.current=123;initial.accumulator=0x40000000;initial.hold=1;initial.pending_release=1;mfrk::AdsrTestAccess::load_explicit_fixture(env,initial);std::vector<Action> actual;
  for(unsigned j=0;j<plan.count;++j){auto a=plan.actions[j];if(a.kind==Kind::main_reset)env.reset();else actual.push_back(a);}const auto x=env.snapshot();differences+=actual!=expected||state.active_count!=after||static_cast<unsigned>(x.phase)!=phase||x.current!=current||x.accumulator!=accumulator||x.hold!=hold||x.pending_release!=pending;
 }
 if(r.p!=r.b.size())return 2;std::cout<<count<<" original shared reset cases, "<<differences<<" differences\n";return differences?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
