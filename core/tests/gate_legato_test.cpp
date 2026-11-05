#include "mfrk/gate_dispatch.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace mfrk::gate_dispatch;
struct Reader{std::vector<std::uint8_t>b;std::size_t p=0;std::uint8_t byte(){if(p>=b.size())throw std::runtime_error("truncated");return b[p++];}std::uint32_t word(){std::uint32_t v=0;for(unsigned i=0;i<4;++i)v|=std::uint32_t(byte())<<(8*i);return v;}};
int main(int argc,char**argv)try{
 if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);Reader r{{std::istreambuf_iterator<char>(f),{}}};
 if(r.b.size()!=4260||mfrk::ui::scene::scope::test::sha256(r.b)!=std::array<std::uint8_t,32>{0xfc,0x4e,0x35,0x82,0x8d,0x0d,0x8f,0x1c,0xaf,0x14,0x8e,0x6d,0x99,0x36,0xf7,0x52,0x9d,0xe9,0xc4,0x2b,0xec,0xaf,0x2f,0x80,0x1f,0xd6,0x98,0xe5,0xf5,0x41,0x18,0xa2})throw std::runtime_error("fixture identity");
 for(char c:std::array<char,8>{'G','A','T','E','L','E','G','1'})if(r.byte()!=c)return 2;auto count=r.word();if(count!=216)return 2;unsigned differences=0;
 for(unsigned i=0;i<count;++i){const auto para=r.byte(),row=r.byte(),status=r.byte(),active=r.byte(),mode=r.byte(),n=r.byte();std::vector<Action> expected;for(unsigned j=0;j<n;++j)expected.push_back({static_cast<Kind>(r.byte()),r.byte(),r.byte(),r.byte()!=0});const auto after=r.byte(),first=r.byte(),last=r.byte();State state;state.active_count=active;state.first_row=row;state.first_sides[row%6]=row/6;Plan plan;
  if(!dispatch(state,row,status,para!=0,mode,false,plan))return 2;std::vector<Action> actual(plan.actions.begin(),plan.actions.begin()+plan.count);differences+=actual!=expected||state.active_count!=after||state.first_row!=first||state.last_row!=last;
 }
 if(r.p!=r.b.size())return 2;std::cout<<count<<" original legato gate cases, "<<differences<<" differences\n";return differences?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
