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
struct Reader {std::vector<std::uint8_t>b;std::size_t p=0;std::uint8_t byte(){if(p>=b.size())throw std::runtime_error("truncated");return b[p++];}std::uint32_t word(){std::uint32_t v=0;for(unsigned i=0;i<4;++i)v|=std::uint32_t(byte())<<(8*i);return v;}};
int main(int argc,char**argv)try{
 if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);Reader r{{std::istreambuf_iterator<char>(f),{}}};
 if(r.b.size()!=3036||mfrk::ui::scene::scope::test::sha256(r.b)!=std::array<std::uint8_t,32>{0x75,0x28,0x72,0xce,0xff,0xd7,0xa0,0xea,0x7e,0x96,0x64,0xe0,0xe9,0x68,0x46,0xf7,0x6b,0x5b,0xe1,0x5e,0x26,0xa9,0x32,0x15,0xaa,0xb6,0xfc,0xfe,0xc3,0x07,0x53,0xfe})throw std::runtime_error("fixture identity");
 for(char c:std::array<char,8>{'G','A','T','E','D','S','P','1'})if(r.byte()!=c)return 2;auto count=r.word();if(count!=144)return 2;unsigned differences=0;
 for(unsigned i=0;i<count;++i){const auto para=r.byte(),row=r.byte(),status=r.byte(),hold=r.byte(),cyclic=r.byte(),active=r.byte(),n=r.byte();std::vector<Action> expected;for(unsigned j=0;j<n;++j)expected.push_back({static_cast<Kind>(r.byte()),r.byte(),r.byte(),r.byte()!=0});const auto phase=r.byte(),pending=r.byte();
  State state;state.active_count=active;state.first_row=row;state.first_sides[row%6]=row/6;Plan plan;if(!dispatch(state,row,status,para!=0,cyclic,false,plan))return 2;
  mfrk::Adsr env;mfrk::AdsrState initial{};initial.phase=mfrk::AdsrPhase::sustain;initial.accumulator=0x40000000;initial.hold=hold;mfrk::AdsrTestAccess::load_explicit_fixture(env,initial);std::vector<Action> observed;
  for(unsigned j=0;j<plan.count;++j){auto action=plan.actions[j];if(action.kind==Kind::main&&!action.on){if(!env.gate(false))return 2;}else observed.push_back(action);}
  differences+=observed!=expected;if(status==2){const auto actual=env.snapshot();differences+=static_cast<unsigned>(actual.phase)!=phase||actual.pending_release!=pending;}
 }
 if(r.p!=r.b.size())return 2;
 std::array<mfrk::notepath::NoteRow,12> rows{};rows[0].bytes[7]=rows[2].bytes[7]=rows[6].bytes[7]=1;State state;RowsPlan all;
 if(!plan_rows(state,rows,false,0,false,all)||state.active_count!=3)return 2;
 const std::vector<Action> multi={{Kind::oscillator,0,0,true},{Kind::oscillator,0,1,true},{Kind::main,0,0,true},{Kind::cyclic,0,0,true},{Kind::oscillator,0,0,true},{Kind::oscillator,0,1,true},{Kind::main,0,1,true},{Kind::cyclic,0,1,true},{Kind::oscillator,2,0,true},{Kind::oscillator,2,1,true},{Kind::main,2,0,true},{Kind::cyclic,2,0,true}};
 if(all.count!=multi.size())return 2;for(unsigned i=0;i<all.count;++i)differences+=!(all.actions[i]==multi[i]);
 rows[4].bytes[7]=6;const auto prior=state;if(plan_rows(state,rows,false,0,false,all)||state.active_count!=prior.active_count)return 2;
 std::cout<<count<<" original gate dispatch cases plus source row order, "<<differences<<" differences\n";return differences?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
