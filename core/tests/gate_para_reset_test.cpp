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
 if(r.b.size()!=1644||mfrk::ui::scene::scope::test::sha256(r.b)!=std::array<std::uint8_t,32>{0x08,0x92,0x89,0x07,0x3b,0x79,0x06,0xc6,0x4a,0x5a,0x08,0x53,0x3f,0xe4,0x5f,0xe3,0xa8,0x6e,0x99,0x88,0x98,0x55,0x05,0xf9,0xc9,0x55,0xbd,0xd4,0x69,0x41,0xd2,0x87})throw std::runtime_error("fixture identity");
 for(char c:std::array<char,8>{'G','A','T','E','P','A','R','1'})if(r.get<char>()!=c)return 2;auto count=r.get<std::uint32_t>();if(count!=48)return 2;unsigned differences=0;
 for(unsigned i=0;i<count;++i){const auto row=r.get<std::uint8_t>(),active=r.get<std::uint8_t>(),other=r.get<std::uint8_t>(),enabled=r.get<std::uint8_t>(),n=r.get<std::uint8_t>();std::vector<Action> expected;for(unsigned j=0;j<n;++j)expected.push_back({static_cast<Kind>(r.get<std::uint8_t>()),r.get<std::uint8_t>(),r.get<std::uint8_t>(),r.get<std::uint8_t>()!=0});const auto after=r.get<std::uint8_t>(),phase=r.get<std::uint8_t>();const auto current=r.get<std::int16_t>();const auto accumulator=r.get<std::int32_t>();const auto hold=r.get<std::uint8_t>(),pending=r.get<std::uint8_t>();
  State state;state.active_count=active;ResetContext reset{other!=0,enabled!=0};Plan plan;if(!dispatch(state,row,5,true,0,false,plan,&reset))return 2;mfrk::Adsr env;mfrk::AdsrState initial{};initial.phase=mfrk::AdsrPhase::sustain;initial.current=123;initial.accumulator=0x40000000;initial.hold=1;initial.pending_release=1;mfrk::AdsrTestAccess::load_explicit_fixture(env,initial);std::vector<Action> actual;
  for(unsigned j=0;j<plan.count;++j){auto a=plan.actions[j];if(a.kind==Kind::main_reset)env.reset();else actual.push_back(a);}const auto x=env.snapshot();differences+=actual!=expected||state.active_count!=after||static_cast<unsigned>(x.phase)!=phase||x.current!=current||x.accumulator!=accumulator||x.hold!=hold||x.pending_release!=pending;
 }
 if(r.p!=r.b.size())return 2;
 std::array<mfrk::notepath::NoteRow,12> rows{};rows[0].bytes[7]=5;rows[6].bytes[0]=1;State state;RowsPlan plan;std::uint16_t mask=1u<<6;
 if(!plan_rows(state,rows,true,0,false,plan,&mask))return 2;for(unsigned i=0;i<plan.count;++i)if(plan.actions[i].kind==Kind::voice_reset)return 2;
 mask=0;if(!plan_rows(state,rows,true,0,false,plan,&mask))return 2;bool found=false;for(unsigned i=0;i<plan.count;++i)found|=plan.actions[i].kind==Kind::voice_reset;if(!found)return 2;
 if(plan_rows(state,rows,true,0,false,plan))return 2;
 std::cout<<count<<" original para reset cases, "<<differences<<" differences\n";return differences?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
