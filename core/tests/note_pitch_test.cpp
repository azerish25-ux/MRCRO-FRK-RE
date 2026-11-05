#include "mfrk/pitch_update.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
using namespace mfrk::pitch_update;
int main(int argc,char**argv){
 if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);std::vector<std::uint8_t> b{std::istreambuf_iterator<char>(f),{}};
 if(b.size()!=908||mfrk::ui::scene::scope::test::sha256(b)!=std::array<std::uint8_t,32>{0x1a,0xa9,0x36,0x19,0x09,0x34,0x7f,0xe4,0x2f,0x51,0xcf,0x48,0xe4,0xf5,0x9e,0xb0,0xfe,0x5a,0xf8,0xb6,0x35,0x4e,0x21,0x23,0x73,0x7a,0x6b,0x57,0xff,0x5c,0xec,0xba}||std::memcmp(b.data(),"NOTEPIT1",8))return 2;
 std::uint32_t count;std::memcpy(&count,b.data()+8,4);if(count!=128)return 2;unsigned differences=0;
 for(unsigned i=0;i<count;++i){const auto at=12+i*7;const auto note=b[at];std::uint16_t expected_word;std::uint32_t expected_target;std::memcpy(&expected_word,b.data()+at+1,2);std::memcpy(&expected_target,b.data()+at+3,4);std::uint16_t word=0;
  State state;GlideControl control;if(!encode_note_pitch(note,word)||!apply_note_pitch(state,control,0,note))return 2;
  differences+=word!=expected_word||std::bit_cast<std::uint32_t>(state.target[0])!=expected_target||state.position[0]!=double(state.target[0])||state.dirty[0]!=1||state.dirty[6]!=1;
 }
 std::uint16_t untouched=42;if(encode_note_pitch(128,untouched)||untouched!=42)return 2;
 std::cout<<count<<" original note conversions, "<<differences<<" differences\n";return differences?1:0;
}
