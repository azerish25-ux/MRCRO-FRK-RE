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
struct Reader {
 std::vector<std::uint8_t> bytes;std::size_t pos=0;
 template<class T>T get(){if(pos+sizeof(T)>bytes.size())throw std::runtime_error("truncated");T value;std::memcpy(&value,bytes.data()+pos,sizeof(T));pos+=sizeof(T);return value;}
 template<class T,std::size_t N>void array(std::array<T,N>& out){for(auto& value:out)value=get<T>();}
};
int main(int argc,char** argv)try{
 static_assert(std::endian::native==std::endian::little);if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);Reader r{{std::istreambuf_iterator<char>(f),{}}};
 if(r.bytes.size()!=60012||mfrk::ui::scene::scope::test::sha256(r.bytes)!=std::array<std::uint8_t,32>{0x09,0x50,0x58,0xf2,0x2b,0x7b,0xcc,0xc1,0xf0,0x1a,0x7e,0x36,0xbb,0x94,0x35,0x2e,0x28,0x48,0x82,0x97,0x66,0xc1,0xf4,0x65,0xd3,0xc8,0x50,0xe5,0xaa,0xd9,0xd0,0x93})throw std::runtime_error("fixture identity");
 for(char c:std::array<char,8>{'P','I','T','C','H','U','P','1'})if(r.get<char>()!=c)return 2;const auto count=r.get<std::uint32_t>();if(count!=96)return 2;unsigned mismatches=0;
 for(unsigned test=0;test<count;++test){
  State state;state.split=r.get<std::uint8_t>();r.array(state.finished);r.array(state.dirty);r.array(state.target);r.array(state.position);r.array(state.increment);state.tune=r.get<float>();state.bend=r.get<float>();r.array(state.side_a);r.array(state.side_b);r.array(state.output);
  for(auto& slot:state.slots){slot.note=r.get<float>();slot.dirty=r.get<std::uint8_t>();}
  std::array<std::uint8_t,12> finished,dirty;r.array(finished);r.array(dirty);std::array<double,12> position;r.array(position);std::array<float,12> output;r.array(output);std::array<Slot,12> slots{};for(auto& slot:slots){slot.note=r.get<float>();slot.dirty=r.get<std::uint8_t>();}
  advance(state);
  for(unsigned i=0;i<12;++i){mismatches+=state.finished[i]!=finished[i]||state.dirty[i]!=dirty[i]||std::bit_cast<std::uint64_t>(state.position[i])!=std::bit_cast<std::uint64_t>(position[i])||std::bit_cast<std::uint32_t>(state.output[i])!=std::bit_cast<std::uint32_t>(output[i])||std::bit_cast<std::uint32_t>(state.slots[i].note)!=std::bit_cast<std::uint32_t>(slots[i].note)||state.slots[i].dirty!=slots[i].dirty;}
 }
 if(r.pos!=r.bytes.size())return 2;std::cout<<count<<" original pitch updates, "<<mismatches<<" differences\n";return mismatches?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
