#include "mfrk/parameter_dispatch.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
namespace {
using namespace mfrk::parameter_dispatch;
struct Boundary {
 ListenerSet source;State& state;std::uint32_t flags;unsigned callbacks=0;
 std::vector<std::uint8_t> events;std::vector<std::pair<std::uint32_t,std::uint32_t>> lookups;
 Boundary(std::uint32_t buckets,State& s,std::uint32_t f,std::size_t n):source{buckets,{}},state(s),flags(f){source.iteration.reserve(n);events.reserve(n*24);}
 const ListenerSet* lookup(std::uint32_t key){lookups.emplace_back(key,state.dispatching);return key==17||key==0x80000011u?&source:nullptr;}
 template<class T>void append(T value){const auto before=events.size();events.resize(before+sizeof(T));std::memcpy(events.data()+before,&value,sizeof(T));}
 void invoke(std::uint64_t identity,std::uint32_t key,std::uint32_t& payload,std::uint32_t origin){
  append(identity);append(key);append(origin);append(std::uint32_t(state.dispatching));append(payload);++callbacks;
  if(callbacks==1){
   if(flags&2)if(!dispatch(state,18,payload,origin,*this))throw std::runtime_error("Nested admission");
   if(flags&4)payload=0x3f000000;
   if(flags&1)source.iteration.clear();
  }
 }
};
}
int main(int argc,char** argv)try{
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=276752)return 2;
 std::vector<std::uint8_t> bytes(276752);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"LDISPLG1",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x65,0x2f,0xcf,0x08,0xd1,0xe0,0xf7,0x2f,0xc2,0xfe,0x97,0x80,0x4a,0xfc,0xc7,0xb0,0x94,0x1b,0xf9,0xe6,0xaf,0xe4,0xc2,0x52,0x69,0xb6,0x6d,0x58,0x37,0x4b,0xad,0x51})return 2;
 std::size_t at=8;auto read=[&]<class T>(){if(at+sizeof(T)>bytes.size())throw std::runtime_error("Fixture bounds");T value;std::memcpy(&value,bytes.data()+at,sizeof(T));at+=sizeof(T);return value;};auto u32=[&](){return read.operator()<std::uint32_t>();};
 const auto cases=u32();const auto base=read.operator()<std::uint64_t>();const auto stride=u32();if(cases!=44||base!=0x900010000ull||stride!=16)return 2;unsigned differences=0,callbacks=0;
 for(unsigned profile=0;profile<cases;++profile){
  const auto buckets=u32(),key=u32(),origin=u32(),prior=u32(),flags=u32(),count=u32();if(prior>1||flags>7||count>16384)return 2;
  State state{prior!=0};Boundary boundary(buckets,state,flags,count);for(unsigned i=0;i<count;++i)boundary.source.iteration.push_back(base+std::uint64_t(stride)*read.operator()<std::uint16_t>());
  const auto expected_count=u32();std::array<std::uint8_t,32> expected_digest;for(auto& byte:expected_digest)byte=read.operator()<std::uint8_t>();
  const auto n=u32();if(n>2)return 2;std::vector<std::pair<std::uint32_t,std::uint32_t>> lookups;for(unsigned i=0;i<n;++i){const auto lookup_key=u32(),busy=u32();lookups.emplace_back(lookup_key,busy);}
  const auto remaining=u32(),busy_after=u32(),payload_after=u32();std::uint32_t payload=0x80000000u;
  const bool accepted=dispatch(state,key,payload,origin,boundary);
  const bool bad=!accepted||boundary.callbacks!=expected_count||mfrk::ui::scene::scope::test::sha256(boundary.events)!=expected_digest||boundary.lookups!=lookups||boundary.source.iteration.size()!=remaining||state.dispatching!=(busy_after!=0)||payload!=payload_after;
  if(bad&&differences++<4)std::cerr<<"Large dispatch mismatch "<<profile<<'\n';
  callbacks+=boundary.callbacks;
 }
 if(at!=bytes.size()||callbacks!=136472)return 2;
 std::cout<<"{\"cases\":"<<cases<<",\"callbacks\":"<<callbacks<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
