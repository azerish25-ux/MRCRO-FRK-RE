#include "mfrk/ui_async_updater.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
using Event=std::array<std::int32_t,8>;
struct Driver;
struct Queue:GuiAsyncUpdateQueue {
 bool allowed=true,seen=false;std::vector<std::shared_ptr<GuiAsyncUpdateMessage>> entries;
 std::weak_ptr<GuiAsyncUpdateMessage> weak;GuiAsyncUpdateMessage* message=nullptr;
 bool post(std::shared_ptr<GuiAsyncUpdateMessage> value)override{seen=true;message=value.get();weak=value;if(!allowed)return false;entries.push_back(std::move(value));return true;}
 void one(){if(entries.empty())return;auto message=std::move(entries.front());entries.erase(entries.begin());message->deliver();}
};
struct Driver:GuiAsyncUpdateTarget {
 Queue queue;std::unique_ptr<GuiAsyncUpdater> updater;
 std::uint8_t flags;GuiAsyncMouseState mouse;unsigned action,calls=0;
 bool alive=true;std::vector<Event> events;
 Driver(std::uint8_t f,std::uint8_t b,bool allowed,unsigned a):flags(f),mouse{b},action(a){queue.allowed=allowed;updater=std::make_unique<GuiAsyncUpdater>(*this,queue);}
 void trigger(){if(alive)gui_component_fake_mouse_move(flags,mouse,*updater);}
 void destroy(){if(alive){updater.reset();alive=false;}}
 void handle_async_update()override{
  if(!alive)throw std::logic_error("Callback after owner destruction");
  ++calls;events.push_back({1,static_cast<std::int32_t>(queue.message->pending.load()),static_cast<std::int32_t>(queue.weak.use_count()),static_cast<std::int32_t>(queue.entries.size()),static_cast<std::int32_t>(calls),0,0,0});
  if(calls==1){if(action==1)trigger();else if(action==2)updater->cancel();else if(action==3)destroy();}
 }
 void apply(std::int32_t op,std::int32_t value){
  if(op==0)trigger();else if(op==1&&alive)updater->cancel();else if(op==2)destroy();else if(op==3)queue.one();else if(op==4)queue.allowed=value!=0;else if(op==5)flags=static_cast<std::uint8_t>(value);else if(op==6)mouse.buttons=static_cast<std::uint8_t>(value);else if(op==7){for(unsigned i=0;i<16&&!queue.entries.empty();++i)queue.one();if(!queue.entries.empty())throw std::logic_error("Async queue did not drain");}
 }
 std::array<std::int32_t,7> snapshot(){
  const auto refs=queue.seen?queue.weak.use_count():alive?1:0;
  const auto pending=refs?(queue.seen?static_cast<std::int32_t>(queue.message->pending.load()):static_cast<std::int32_t>(updater->pending_word().load())):-1;
  return {alive,refs!=0,pending,static_cast<std::int32_t>(refs),static_cast<std::int32_t>(queue.entries.size()),static_cast<std::int32_t>(calls),queue.allowed};
 }
};
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;constexpr Identity expected={"async-updates","ASYNC001",25316,64,4075173815u,{0x82,0x9,0x6b,0x14,0x4,0xdb,0x2a,0x87,0x9d,0xd1,0xc8,0x15,0x97,0x51,0xf0,0xc1,0xc7,0x7d,0xe9,0x87,0x62,0x28,0xc7,0xb8,0x6b,0xb5,0xf,0xa6,0xa2,0x35,0x29,0x1d}};
 auto reader=Reader::file(argv[1],expected);unsigned differences=0,step_count=0,callback_count=0;
 for(unsigned i=0;i<expected.count;++i){
  const auto flags=reader.u32(),buttons=reader.u32();const auto allowed=reader.flag();const auto action=reader.u32(),steps=reader.u32();Driver test(static_cast<std::uint8_t>(flags),static_cast<std::uint8_t>(buttons),allowed,action);
  for(unsigned s=0;s<steps;++s){const auto op=reader.i32(),value=reader.i32();std::array<std::int32_t,7> native;for(auto& v:native)v=reader.i32();const auto count=reader.u32();std::vector<Event> events(count);for(auto& e:events)for(auto& v:e)v=reader.i32();test.events.clear();test.apply(op,value);const auto actual=test.snapshot();if(actual!=native||test.events!=events){if(differences++<4){std::cerr<<"Async mismatch "<<i<<':'<<s<<" op="<<op<<" state="<<(actual!=native)<<" callbacks="<<test.events.size()<<'/'<<events.size()<<'\n';for(unsigned n=0;n<7;++n)if(actual[n]!=native[n])std::cerr<<" state "<<n<<' '<<actual[n]<<'/'<<native[n]<<'\n';}}++step_count;callback_count+=count;}
 }
 reader.end();if(step_count!=547||callback_count!=67)return 2;std::cout<<"{\"cases\":"<<expected.count<<",\"steps\":"<<step_count<<",\"callbacks\":"<<callback_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
