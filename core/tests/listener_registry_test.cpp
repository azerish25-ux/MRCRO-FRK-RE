#include "mfrk/listener_registry.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
namespace {
using namespace mfrk::parameter_dispatch;
struct Event {std::uint32_t id;std::uint64_t notifier;std::uint32_t destroying,size;bool operator==(const Event&)const=default;};
struct Boundary:ListenerRegistryBoundary {
 ListenerRegistry* owner=nullptr;std::vector<Event> events;bool bad=false;
 Boundary(){events.reserve(2048);}
 void remove(std::uint64_t notifier,std::uint32_t id,std::uint64_t listener)noexcept override{
  if(listener!=42||!owner){bad=true;return;}
  events.push_back({id,notifier,owner->destroying()?1u:0u,static_cast<std::uint32_t>(owner->size())});
  if(owner->find(id)!=notifier||owner->erase(id))bad=true;
 }
};
}
int main(int argc,char** argv)try{
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=221628)return 2;
 std::vector<std::uint8_t> bytes(221628);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"LREGSRC1",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x3d,0x40,0x1e,0x09,0x9c,0xd2,0x82,0x83,0x6d,0x5c,0x36,0x97,0x6e,0xe6,0x85,0x51,0xd2,0xcb,0xd2,0x77,0x13,0x6e,0x21,0x4d,0xc4,0x11,0x3d,0x51,0x94,0x06,0xe5,0x00})return 2;
 std::size_t at=8;auto read=[&]<class T>(){if(at+sizeof(T)>bytes.size())throw std::runtime_error("Fixture bounds");T v;std::memcpy(&v,bytes.data()+at,sizeof(T));at+=sizeof(T);return v;};
 auto u32=[&](){return read.operator()<std::uint32_t>();};auto u64=[&](){return read.operator()<std::uint64_t>();};const auto count=u32();unsigned differences=0,operations=0,erasures=0,callbacks=0;
 auto snapshot=[&](const ListenerRegistry& registry){
  const auto buckets=u32(),destroying=u32(),n=u32();if(n>2048)throw std::runtime_error("Snapshot count");std::vector<Registration> expected;expected.reserve(n);
  for(unsigned i=0;i<n;++i){const auto id=u32();const auto notifier=u64();expected.push_back({id,notifier});}
  const std::vector<Registration> actual(registry.registrations().begin(),registry.registrations().end());
  differences+=registry.bucket_count()!=buckets||registry.destroying()!=(destroying!=0)||actual!=expected;
 };
 for(unsigned profile=0;profile<count;++profile){
  Boundary boundary;ListenerRegistry registry(42,boundary);boundary.owner=&registry;snapshot(registry);
  const auto n=u32();for(unsigned i=0;i<n;++i){const auto id=u32();const auto notifier=u64();const auto inserted=u32();const auto found=u64();const auto has_snapshot=u32();differences+=registry.insert(id,notifier)!=(inserted!=0)||registry.find(id)!=found;if(has_snapshot)snapshot(registry);++operations;}
  const auto removals=u32();for(unsigned i=0;i<removals;++i){const auto id=u32(),erased=u32();const auto found=u64();differences+=registry.erase(id)!=(erased!=0)||registry.find(id)!=found;snapshot(registry);++erasures;}
  snapshot(registry);const auto m=u32();std::vector<Event> expected;expected.reserve(m);
  for(unsigned i=0;i<m;++i){const auto id=u32();const auto notifier=u64();const auto destroying=u32(),size=u32();expected.push_back({id,notifier,destroying,size});}
  registry.close();differences+=boundary.bad||boundary.events!=expected||registry.size()!=0||!registry.destroying();callbacks+=m;
  registry.close();differences+=boundary.events.size()!=m;
 }
 if(at!=bytes.size()||count!=4||operations!=1187||erasures!=19||callbacks!=1044)return 2;
 std::cout<<"{\"cases\":"<<count<<",\"operations\":"<<operations<<",\"erasures\":"<<erasures<<",\"close_callbacks\":"<<callbacks<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
