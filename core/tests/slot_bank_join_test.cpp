#include "mfrk/core_bank.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace mfrk;
struct Reader {std::vector<std::uint8_t>b;std::size_t p=0;template<class T>T get(){if(p+sizeof(T)>b.size())throw std::runtime_error("truncated");T v;std::memcpy(&v,b.data()+p,sizeof v);p+=sizeof v;return v;}template<class T,std::size_t N>void array(std::array<T,N>&a){for(auto&v:a)v=get<T>();}};
int main(int argc,char**argv)try{
 if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);Reader r{{std::istreambuf_iterator<char>(f),{}}};
 if(r.b.size()!=12108||ui::scene::scope::test::sha256(r.b)!=std::array<std::uint8_t,32>{0xd4,0x04,0x2d,0x71,0xb5,0x59,0xac,0x7f,0x21,0x4d,0xaa,0x99,0xe2,0xff,0xfe,0x5d,0xf1,0x31,0x61,0x5e,0xc5,0x13,0x58,0xd4,0x3f,0x61,0x1d,0xca,0xac,0x3e,0xbd,0xe3})throw std::runtime_error("fixture identity");
 for(char c:std::array<char,8>{'S','L','O','T','B','N','K','1'})if(r.get<char>()!=c)return 2;auto count=r.get<std::uint32_t>();if(count!=64)return 2;
 auto tables=std::make_unique<core_bank::Tables>();karplus::SharedRandom random{54321};auto side=std::make_unique<core_bank::Side>(*tables,random,0x400020000ull);core_bank::SlotController slot(*side);unsigned differences=0;
 for(unsigned n=0;n<count;++n){std::array<std::uint16_t,3> words;std::array<std::uint8_t,3> dirty;std::array<std::uint16_t,9> pitch;std::array<std::uint8_t,9> pitch_dirty;r.array(words);r.array(dirty);r.array(pitch);r.array(pitch_dirty);pitch_update::Slot base;base.note=std::bit_cast<float>(r.get<std::uint32_t>());base.dirty=r.get<std::uint8_t>();auto trigger=r.get<std::uint8_t>(),updated=r.get<std::uint8_t>();auto note=r.get<std::uint32_t>(),hz=r.get<std::uint32_t>();std::array<std::uint8_t,9> expected_dirty;r.array(expected_dirty);auto expected_force=r.get<std::uint8_t>();std::array<std::uint32_t,32> expected;r.array(expected);
  if(trigger!=255)side->trigger(slot.selected(),trigger);for(unsigned i=0;i<3;++i)if(!slot.set_model_control(i,words[i],dirty[i]))return 2;for(unsigned i=0;i<9;++i)if(!slot.set_pitch_control(i,pitch[i],pitch_dirty[i]))return 2;slot_pitch::Result result{};if(!slot.prepare(base,result))return 2;
  differences+=result.recomputed!=(updated!=0)||std::bit_cast<std::uint32_t>(result.note)!=note||std::bit_cast<std::uint32_t>(result.hz)!=hz||slot.pitch_dirty()!=expected_dirty||base.dirty!=expected_force;
  std::array<float,32> audio{};if(!side->render(slot.selected(),audio))return 2;for(unsigned i=0;i<32;++i)differences+=std::bit_cast<std::uint32_t>(audio[i])!=expected[i];
 }
 if(r.p!=r.b.size())return 2;std::cout<<count<<" original slot/bank blocks, "<<differences<<" differences\n";return differences?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
