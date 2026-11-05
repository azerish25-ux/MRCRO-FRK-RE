#include "mfrk/publication_sink.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace mfrk::publication_sink;
struct Reader {std::vector<std::uint8_t> b;std::size_t p=0;template<class T>T get(){if(p+sizeof(T)>b.size())throw std::runtime_error("truncated");T x;std::memcpy(&x,b.data()+p,sizeof x);p+=sizeof x;return x;}template<std::size_t N>std::array<std::uint8_t,N> array(){std::array<std::uint8_t,N> a{};for(auto& x:a)x=get<std::uint8_t>();return a;}};
int main(int argc,char**argv)try{
 static_assert(std::endian::native==std::endian::little);if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);Reader r{{std::istreambuf_iterator<char>(f),{}}};
 if(r.b.size()!=6060||mfrk::ui::scene::scope::test::sha256(r.b)!=std::array<std::uint8_t,32>{0x34,0xc4,0xd8,0x5a,0x45,0x67,0xba,0x6c,0x2b,0x7f,0x92,0xaa,0x61,0xd3,0x1e,0x4e,0x5e,0x23,0xa7,0x11,0xac,0x06,0x32,0x25,0xec,0xdf,0x62,0x22,0x5e,0x6f,0x35,0x83})throw std::runtime_error("fixture identity");
 for(char c:std::array<char,8>{'N','O','T','E','S','N','K','1'})if(r.get<char>()!=c)return 2;const auto count=r.get<std::uint32_t>();if(count!=48)return 2;unsigned differences=0;
 for(unsigned test=0;test<count;++test){const auto index=r.get<std::uint8_t>();Owner owner;owner.count=r.get<std::uint32_t>();mfrk::notepath::NoteRow row;row.bytes=r.array<20>();owner.rows[index].bytes=r.array<20>();const auto slot=owner.count<256?owner.count:255;owner.queue[slot]=r.array<24>();owner.last_active=r.get<std::uint32_t>();const auto tail=r.get<std::uint8_t>();const auto expected_row=r.array<20>();const auto expected_count=r.get<std::uint32_t>(),expected_last=r.get<std::uint32_t>();const auto expected_record=r.array<24>();
  if(!publish(owner,index,row,tail))return 2;differences+=owner.rows[index].bytes!=expected_row||owner.count!=expected_count||owner.last_active!=expected_last||owner.queue[slot]!=expected_record;
 }
 if(r.p!=r.b.size())return 2;
 Owner owner;owner.count=7;owner.queue[0].fill(0xa5);for(auto& row:owner.rows)row.bytes.fill(0x5a);
 begin_block(owner);finish_voice_block(owner);if(owner.count!=0||owner.queue[0][0]!=0xa5)return 2;
 for(const auto& row:owner.rows)for(unsigned i=0;i<20;++i)if(row.bytes[i]!=(i==7?0:0x5a))return 2;
 std::cout<<count<<" original publication sinks, "<<differences<<" differences\n";return differences?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
