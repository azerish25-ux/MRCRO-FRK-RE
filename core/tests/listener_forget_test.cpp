#include "mfrk/listener_registry.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
namespace {
using namespace mfrk::parameter_dispatch;
struct Snapshot {std::uint32_t buckets,destroying;std::vector<Registration> rows;};
bool matches(const ListenerRegistry& owner,const Snapshot& expected){
 return owner.bucket_count()==expected.buckets&&owner.destroying()==(expected.destroying!=0)&&std::vector<Registration>(owner.registrations().begin(),owner.registrations().end())==expected.rows;
}
struct Event {std::uint32_t id;std::uint64_t notifier;std::uint32_t destroying,size;bool operator==(const Event&)const=default;};
struct Boundary:ListenerRegistryBoundary {
 ListenerRegistry* owner=nullptr;const Snapshot* guarded=nullptr;bool bad=false;std::vector<Event> events;
 Boundary(){events.reserve(2048);}
 void remove(std::uint64_t notifier,std::uint32_t id,std::uint64_t listener)noexcept override{
  if(!owner||listener!=42){bad=true;return;}
  if(events.empty()&&guarded&&!matches(*owner,*guarded))bad=true;
  owner->forget_notifier(0x700000000ull);
  if(events.empty()&&guarded&&!matches(*owner,*guarded))bad=true;
  events.push_back({id,notifier,owner->destroying()?1u:0u,static_cast<std::uint32_t>(owner->size())});
 }
};
}
int main(int argc,char** argv)try{
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=53580)return 2;
 std::vector<std::uint8_t> bytes(53580);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"LREGFOR1",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xaa,0x6d,0xc1,0xee,0x66,0x2d,0x21,0xbe,0xb2,0xa7,0x72,0xa8,0x28,0xeb,0x6a,0x2d,0xe6,0x12,0x79,0x5e,0x37,0x1a,0x8a,0x94,0x5e,0xd4,0xd0,0x91,0xe8,0xd3,0xc0,0xb9})return 2;
 std::size_t at=8;auto read=[&]<class T>(){if(at+sizeof(T)>bytes.size())throw std::runtime_error("Fixture bounds");T v;std::memcpy(&v,bytes.data()+at,sizeof(T));at+=sizeof(T);return v;};auto u32=[&](){return read.operator()<std::uint32_t>();};auto u64=[&](){return read.operator()<std::uint64_t>();};
 auto snapshot=[&](){Snapshot s{u32(),u32(),{}};const auto count=u32();if(count>2048)throw std::runtime_error("Snapshot count");for(unsigned i=0;i<count;++i){const auto id=u32();const auto notifier=u64();s.rows.push_back({id,notifier});}return s;};
 const auto cases=u32();unsigned registrations=0,forgets=0,callbacks=0,differences=0;
 for(unsigned profile=0;profile<cases;++profile){
  Snapshot guarded{};Boundary boundary;ListenerRegistry registry(42,boundary);boundary.owner=&registry;
  const auto count=u32();for(unsigned i=0;i<count;++i){const auto id=u32();const auto notifier=u64();differences+=!registry.insert(id,notifier);++registrations;}
  differences+=!matches(registry,snapshot());const auto n=u32();
  for(unsigned i=0;i<n;++i){const auto notifier=u64();registry.forget_notifier(notifier);differences+=!matches(registry,snapshot())||!boundary.events.empty();++forgets;}
  guarded=snapshot();boundary.guarded=&guarded;differences+=!matches(registry,snapshot());const auto m=u32();std::vector<Event> expected;
  for(unsigned i=0;i<m;++i){const auto id=u32();const auto notifier=u64();const auto destroying=u32(),size=u32();expected.push_back({id,notifier,destroying,size});}
  registry.close();differences+=boundary.bad||boundary.events!=expected;callbacks+=m;
 }
 if(at!=bytes.size()||cases!=4||registrations!=593||forgets!=16||callbacks!=410)return 2;
 std::cout<<"{\"cases\":"<<cases<<",\"registrations\":"<<registrations<<",\"forget_calls\":"<<forgets<<",\"close_callbacks\":"<<callbacks<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
