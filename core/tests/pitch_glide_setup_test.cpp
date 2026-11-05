#include "mfrk/pitch_update.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace mfrk::pitch_update;
struct Reader {std::vector<std::uint8_t> b;std::size_t p=0;template<class T>T get(){if(p+sizeof(T)>b.size())throw std::runtime_error("truncated");T x;std::memcpy(&x,b.data()+p,sizeof x);p+=sizeof x;return x;}template<class T,std::size_t N>void array(std::array<T,N>& a){for(auto& x:a)x=get<T>();}};
int main(int argc,char** argv)try{
 static_assert(std::endian::native==std::endian::little);if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);Reader r{{std::istreambuf_iterator<char>(f),{}}};
 if(r.b.size()!=163084||mfrk::ui::scene::scope::test::sha256(r.b)!=std::array<std::uint8_t,32>{0x0e,0xa1,0xb9,0x8e,0x38,0x29,0x57,0xb5,0x71,0xcb,0x6e,0x86,0xc1,0xc0,0xaf,0x5a,0x23,0x43,0x39,0xea,0x4a,0x76,0xd0,0x44,0xeb,0x5c,0x55,0xe7,0x51,0x9d,0xe6,0x12})throw std::runtime_error("fixture identity");
 for(char c:std::array<char,8>{'P','I','T','C','H','G','L','1'})if(r.get<char>()!=c)return 2;const auto count=r.get<std::uint32_t>();if(count!=256)return 2;unsigned differences=0;
 for(unsigned test=0;test<count;++test){
  const auto index=r.get<std::uint8_t>(),copy=r.get<std::uint8_t>();State state;GlideControl control;state.split=r.get<std::uint8_t>();control.scale_mode=r.get<std::uint8_t>();control.gate_mode=r.get<std::uint8_t>();control.shared_gate=r.get<std::uint8_t>();control.alternate=r.get<std::uint8_t>();control.force=r.get<std::uint8_t>();control.last_index=r.get<std::uint32_t>();r.array(control.enabled);r.array(control.held);r.array(control.dirty_a);r.array(control.dirty_b);r.array(state.finished);r.array(state.dirty);r.array(state.target);r.array(control.rate_a);r.array(control.rate_b);r.array(state.position);r.array(state.increment);
  const auto expected_gate=r.get<std::uint8_t>();std::array<std::uint8_t,12> finished,dirty;r.array(finished);r.array(dirty);std::array<double,12> position,increment;r.array(position);r.array(increment);
  if(!setup_glide(state,control,index,copy!=0))throw std::runtime_error("valid setup rejected");differences+=control.shared_gate!=expected_gate;
  for(unsigned i=0;i<12;++i)differences+=state.finished[i]!=finished[i]||state.dirty[i]!=dirty[i]||std::bit_cast<std::uint64_t>(state.position[i])!=std::bit_cast<std::uint64_t>(position[i])||std::bit_cast<std::uint64_t>(state.increment[i])!=std::bit_cast<std::uint64_t>(increment[i]);
 }
 if(r.p!=r.b.size())return 2;std::cout<<count<<" original glide setups, "<<differences<<" differences\n";return differences?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
