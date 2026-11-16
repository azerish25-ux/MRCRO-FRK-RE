#include "mfrk/parameter_notifier.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
namespace {
using namespace mfrk::parameter_dispatch;
template<class T>void append(std::vector<std::uint8_t>& out,T value){const auto p=out.size();out.resize(p+sizeof(T));std::memcpy(out.data()+p,&value,sizeof(T));}
using Notifiers=std::vector<std::unique_ptr<ParameterNotifier>>;
using Listeners=std::vector<std::unique_ptr<ListenerRegistry>>;
std::array<std::uint8_t,32> state_digest(Notifiers& notifiers,const Listeners& listeners,const std::vector<bool>& closed,const std::vector<std::uint32_t>& keys){
 std::vector<std::uint8_t> out;
 for(auto& notifier:notifiers){
  append(out,std::uint8_t(notifier->dispatching()));append(out,std::uint8_t(notifier->destroying()));
  for(unsigned slot=0;slot<6;++slot){
   std::vector<std::pair<std::uint32_t,const ListenerSet*>> rows;
   for(auto key:keys)if((key&0x7fffffffu)/1000000==slot%3)if(const auto* set=notifier->members(key,slot>=3))rows.emplace_back(key,set);
   append(out,static_cast<std::uint32_t>(rows.size()));
   for(const auto& [key,set]:rows){append(out,key);append(out,set->buckets);append(out,static_cast<std::uint32_t>(set->iteration.size()));for(auto identity:set->iteration)append(out,identity);}
  }
 }
 for(unsigned i=0;i<listeners.size();++i){
  append(out,std::uint8_t(closed[i]));
  if(!closed[i]){const auto& owner=*listeners[i];append(out,static_cast<std::uint32_t>(owner.bucket_count()));append(out,std::uint32_t(owner.destroying()));append(out,static_cast<std::uint32_t>(owner.size()));for(const auto& row:owner.registrations()){append(out,row.id);append(out,row.notifier);}}
 }
 return mfrk::ui::scene::scope::test::sha256(out);
}
struct Callback {
 ParameterNotifier* current=nullptr;std::uint32_t flags=0;unsigned count=0;std::vector<std::uint8_t> events;
 void reset(ParameterNotifier& notifier,std::uint32_t mode){current=&notifier;flags=mode;count=0;events.clear();}
 void invoke(std::uint64_t identity,std::uint32_t key,std::uint32_t& payload,std::uint32_t origin){
  append(events,identity);append(events,key);append(events,origin);append(events,std::uint32_t(current->dispatching()));append(events,payload);append(events,std::uint32_t(current->destroying()));++count;
  if(count==1){
   if(flags&1)current->remove(key,identity);
   if(flags&2)if(!current->notify(983040,payload,origin,*this))throw std::runtime_error("Nested notification rejected");
   if(flags&4)payload=0x3f000000;
   if(flags&8)current->close();
  }
 }
};
}
int main(int argc,char** argv)try{
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=55400)return 2;
 std::vector<std::uint8_t> bytes(55400);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"PNOTOWN1",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xfb,0xab,0x0c,0x6a,0x3f,0xcf,0x44,0x31,0xf9,0xe8,0xa9,0xf0,0xf7,0x1d,0xd2,0xba,0x95,0x99,0x30,0x9e,0xb6,0xf3,0xea,0x5a,0xad,0x73,0x58,0x1e,0x01,0xff,0x88,0xdf})return 2;
 std::size_t at=8;auto read=[&]<class T>(){if(at+sizeof(T)>bytes.size())throw std::runtime_error("Fixture bounds");T value;std::memcpy(&value,bytes.data()+at,sizeof(T));at+=sizeof(T);return value;};auto u32=[&](){return read.operator()<std::uint32_t>();};auto u64=[&](){return read.operator()<std::uint64_t>();};auto digest=[&](){std::array<std::uint8_t,32> result;for(auto& byte:result)byte=read.operator()<std::uint8_t>();return result;};
 const auto cases=u32();unsigned differences=0,steps=0,callbacks=0;
 for(unsigned profile=0;profile<cases;++profile){
  const auto count=u32(),contexts=u32();if(count>1024||contexts!=2)return 2;
  NotifierDirectory directory;Notifiers notifiers;for(unsigned i=0;i<contexts;++i)notifiers.push_back(std::make_unique<ParameterNotifier>(u64(),directory));
  const auto base=u64();const auto stride=u32(),key_count=u32();if(stride!=256||key_count>64)return 2;
  std::vector<std::uint32_t> keys;for(unsigned i=0;i<key_count;++i)keys.push_back(u32());
  Listeners listeners;std::vector<bool> closed(count,false);for(unsigned i=0;i<count;++i)listeners.push_back(std::make_unique<ListenerRegistry>(base+std::uint64_t(stride)*i,directory));
  const auto initial=digest();differences+=state_digest(notifiers,listeners,closed,keys)!=initial;const auto n=u32();Callback callback;
  for(unsigned step=0;step<n;++step){
   const auto kind=u32(),context=u32(),actor=u32(),key=u32(),flags=u32();const auto expected_state=digest();const auto expected_callbacks=u32();const auto expected_events=digest();const auto expected_payload=u32();if(context>=contexts||kind>4)return 2;
   auto& notifier=*notifiers[context];callback.reset(notifier,flags);std::uint32_t payload=0x80000000u;
   if(kind==0){if(actor!=0xffffffffu&&actor>=count)return 2;notifier.add(key,actor==0xffffffffu?nullptr:listeners[actor].get(),flags!=0);}
   else if(kind==1){if(actor>=count)return 2;notifier.remove(key,listeners[actor]->identity());}
   else if(kind==2){if(actor>=count)return 2;listeners[actor]->close();closed[actor]=true;}
   else if(kind==3)notifier.close();
   else if(!notifier.notify(key,payload,actor,callback))throw std::runtime_error("Notification rejected");
   const bool bad=state_digest(notifiers,listeners,closed,keys)!=expected_state||callback.count!=expected_callbacks||mfrk::ui::scene::scope::test::sha256(callback.events)!=expected_events||(kind==4?payload:0)!=expected_payload;
   if(bad&&differences++<8)std::cerr<<"Owned notifier mismatch profile "<<profile<<" step "<<step<<'\n';
   ++steps;callbacks+=callback.count;
  }
 }
 if(at!=bytes.size()||cases!=2||steps!=600||callbacks!=3141)return 2;
 std::cout<<"{\"cases\":"<<cases<<",\"steps\":"<<steps<<",\"callbacks\":"<<callbacks<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
