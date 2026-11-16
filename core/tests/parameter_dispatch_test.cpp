#include "mfrk/parameter_dispatch.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
using namespace mfrk::parameter_dispatch;
struct Event {std::uint64_t identity;std::uint32_t key,origin,busy,bits;bool operator==(const Event&)const=default;};
struct Boundary {
 ListenerSet source;State& state;std::uint8_t flags;
 std::vector<Event> events;std::vector<std::pair<std::uint32_t,std::uint32_t>> lookups;
 const ListenerSet* lookup(std::uint32_t key){lookups.emplace_back(key,state.dispatching);return key==17||key==0x80000011u?&source:nullptr;}
 void invoke(std::uint64_t identity,std::uint32_t key,std::uint32_t& payload,std::uint32_t origin){
  events.push_back({identity,key,origin,std::uint32_t(state.dispatching),payload});
  if(events.size()==1){
   if(flags&1)source.iteration.clear();
   if(flags&2)if(!dispatch(state,18,payload,origin,*this))throw std::runtime_error("nested admission");
   if(flags&4)payload=0x3f000000u;
  }
 }
};
int main(int argc,char**argv)try{
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=100892)return 2;
 std::vector<std::uint8_t> bytes(100892);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"LDISPV01",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xc0,0xf9,0x65,0xfa,0x67,0xdc,0x28,0x66,0x3d,0x0f,0x44,0x97,0x21,0x63,0x02,0xe6,0xe4,0x56,0xa6,0x4f,0x76,0x93,0xa4,0x83,0x46,0xc3,0x16,0x9b,0xca,0xc0,0xda,0x79})return 2;
 std::size_t position=8;auto read=[&]<class T>(){if(position>bytes.size()||sizeof(T)>bytes.size()-position)throw std::runtime_error("truncated fixture");T result;std::memcpy(&result,bytes.data()+position,sizeof result);position+=sizeof result;return result;};
 if(read.operator()<std::uint32_t>()!=134)return 2;
 unsigned differences=0,callbacks=0;
 for(unsigned ordinal=0;ordinal<134;++ordinal){
  const auto buckets=read.operator()<std::uint16_t>();const auto key=read.operator()<std::uint32_t>(),origin=read.operator()<std::uint32_t>();const auto prior=read.operator()<std::uint8_t>(),flags=read.operator()<std::uint8_t>();const auto count=read.operator()<std::uint16_t>();if(prior>1||flags>7||count>128)return 2;
  State state{bool(prior)};Boundary boundary{{buckets,{}},state,flags,{},{}};for(unsigned i=0;i<count;++i)boundary.source.iteration.push_back(read.operator()<std::uint64_t>());
  const auto events=read.operator()<std::uint16_t>();if(events>128)return 2;std::vector<Event> expected;
  for(unsigned i=0;i<events;++i){const auto identity=read.operator()<std::uint64_t>();const auto event_key=read.operator()<std::uint32_t>(),event_origin=read.operator()<std::uint32_t>();const auto busy=read.operator()<std::uint8_t>();const auto bits=read.operator()<std::uint32_t>();expected.push_back({identity,event_key,event_origin,busy,bits});}
  const auto lookups=read.operator()<std::uint16_t>();if(lookups>2)return 2;std::vector<std::pair<std::uint32_t,std::uint32_t>> expected_lookups;for(unsigned i=0;i<lookups;++i){const auto lookup_key=read.operator()<std::uint32_t>();const auto busy=read.operator()<std::uint8_t>();expected_lookups.emplace_back(lookup_key,busy);}
  const auto remaining=read.operator()<std::uint16_t>();const auto busy_after=read.operator()<std::uint8_t>();const auto payload_after=read.operator()<std::uint32_t>();
  std::uint32_t payload=0x80000000u;const bool accepted=dispatch(state,key,payload,origin,boundary);callbacks+=boundary.events.size();
  differences+=!accepted||boundary.events!=expected||boundary.lookups!=expected_lookups||boundary.source.iteration.size()!=remaining||state.dispatching!=bool(busy_after)||payload!=payload_after;
 }
 if(position!=bytes.size()||callbacks!=3336)return 2;
 for(const ListenerSet& bad:std::vector<ListenerSet>{{7,{}},{9,{}},{131072,{}},{8,{0}},{8,{1,1}},{8,std::vector<std::uint64_t>(9,1)}})if(snapshot(bad))return 3;
 std::cout<<"{\"cases\":134,\"callbacks\":"<<callbacks<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what();return 2;}
