#include "mfrk/note_row_pitch.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace mfrk;
struct Reader {std::vector<std::uint8_t>b;std::size_t p=0;template<class T>T get(){if(p+sizeof(T)>b.size())throw std::runtime_error("truncated");T v;std::memcpy(&v,b.data()+p,sizeof v);p+=sizeof v;return v;}template<class T,std::size_t N>void array(std::array<T,N>& a){for(auto&v:a)v=get<T>();}};
int main(int argc,char**argv)try{
 if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);Reader r{{std::istreambuf_iterator<char>(f),{}}};
 if(r.b.size()!=32748||ui::scene::scope::test::sha256(r.b)!=std::array<std::uint8_t,32>{0xfc,0xa1,0x4e,0x96,0x6a,0x33,0x66,0xa8,0xf6,0xf0,0x1a,0x83,0xe2,0xa0,0xdb,0x70,0x9d,0x89,0x1c,0xc4,0xd0,0xa0,0x3e,0x8d,0xfc,0x0d,0x31,0x22,0x13,0x31,0x04,0x4f})throw std::runtime_error("fixture identity");
 for(char c:std::array<char,8>{'R','O','W','P','I','T','0','1'})if(r.get<char>()!=c)return 2;auto count=r.get<std::uint32_t>();if(count!=96)return 2;unsigned differences=0;
 for(unsigned n=0;n<count;++n){const auto para=r.get<std::uint8_t>();auto active=r.get<std::uint8_t>();std::array<notepath::NoteRow,12> rows{};for(auto&row:rows)r.array(row.bytes);note_row_pitch::Records records;r.array(records.words);pitch_update::GlideControl control;r.array(control.dirty_a);r.array(control.held);control.shared_gate=r.get<std::uint8_t>();const auto expected_active=r.get<std::uint8_t>();std::array<std::uint16_t,12> words;r.array(words);std::array<std::uint8_t,12> dirty,held;r.array(dirty);r.array(held);const auto expected_gate=r.get<std::uint8_t>();
  if(!note_row_pitch::collect(records,control,active,rows,para!=0))return 2;differences+=active!=expected_active||records.words!=words||control.dirty_a!=dirty||control.held!=held||control.shared_gate!=expected_gate;
 }
 if(r.p!=r.b.size())return 2;
 std::array<notepath::NoteRow,12> rows{};rows[0].bytes[1]=60;rows[0].bytes[7]=1;note_row_pitch::Records records;pitch_update::GlideControl control;pitch_update::State state;std::uint8_t active=0;
 if(!note_row_pitch::collect(records,control,active,rows,false)||!note_row_pitch::apply_pending(records,state,control))return 2;
 pitch_update::advance(state);if(active!=1||state.slots[0].note!=60.f||state.slots[1].note!=60.f||!state.slots[0].dirty||!state.slots[1].dirty||!control.dirty_a[0])return 2;
 note_row_pitch::clear_note_dirty(control);if(control.dirty_a[0])return 2;
 const auto saved=records.words;rows[1].bytes[7]=6;if(note_row_pitch::collect(records,control,active,rows,false)||records.words!=saved||active!=1)return 2;
 rows[1].bytes[7]=0;control.dirty_a[6]=1;const auto old_target=state.target;if(note_row_pitch::apply_pending(records,state,control)||state.target!=old_target||!control.dirty_a[6])return 2;
 std::cout<<count<<" original row/pitch cases, "<<differences<<" differences\n";return differences?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
