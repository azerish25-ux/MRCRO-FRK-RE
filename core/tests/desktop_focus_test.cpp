#include "mfrk/ui_desktop_focus.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
using Event=std::array<std::int32_t,8>;
struct Queue:GuiAsyncUpdateQueue {
 std::vector<std::shared_ptr<GuiAsyncUpdateMessage>> entries;
 bool post(std::shared_ptr<GuiAsyncUpdateMessage> message)override{entries.push_back(std::move(message));return true;}
 void one(){if(entries.empty())return;auto message=std::move(entries.front());entries.erase(entries.begin());message->deliver();}
};
struct Driver;
struct Listener:GuiGlobalFocusListener {Driver* driver=nullptr;unsigned id=0;void global_focus_changed(GuiVisibilityComponent*)override;};
struct Driver {
 std::array<std::int32_t,7> config;
 GuiComponentFocusState state;std::array<GuiVisibilityComponent,2> nodes;
 std::array<Listener,5> objects;std::vector<GuiGlobalFocusListener*> listeners;
 Queue queue;GuiDesktopFocusUpdates updates{state,listeners,queue};
 unsigned notified=0;std::vector<Event> events;
 explicit Driver(const std::array<std::int32_t,7>& c):config(c){for(unsigned i=0;i<5;++i){objects[i].driver=this;objects[i].id=i;}for(int i=0;i<c[3];++i)listeners.push_back(&objects[static_cast<unsigned>(i)]);state.focused=c[2]<0?nullptr:&nodes[static_cast<unsigned>(c[2])];}
 std::int32_t index(GuiVisibilityComponent* p){if(!p)return -1;for(unsigned i=0;i<2;++i)if(p==&nodes[i])return static_cast<std::int32_t>(i);throw std::logic_error("Focus snapshot identity");}
 void notification(unsigned id,GuiVisibilityComponent* p){
  ++notified;events.push_back({1,static_cast<std::int32_t>(id),index(p),index(state.focused),static_cast<std::int32_t>(state.live_update_pending().load()),static_cast<std::int32_t>(listeners.size()),0,0});
  if(notified==1){
   const auto action=config[4];
   if(action==1&&p){p->retire();state.focused=nullptr;}
   else if(action==2)state.focused=config[5]<0?nullptr:&nodes[static_cast<unsigned>(config[5])];
   else if(action==3)listeners.erase(std::find(listeners.begin(),listeners.end(),&objects[id]));
   else if(action==4)listeners.clear();
   else if(action==5){if(std::find(listeners.begin(),listeners.end(),&objects[4])==listeners.end())listeners.push_back(&objects[4]);}
   else if(action==6){if(!listeners.empty())listeners.erase(listeners.begin());}
   if(config[6])updates.trigger();
  }
 }
 void first(){if(config[0]==0){state.live_update_pending().store(static_cast<std::uint32_t>(config[1]));updates.handle_async_update();}else{updates.trigger();if(!config[1])updates.cancel();queue.one();}}
 void drain(){for(unsigned i=0;i<16&&!queue.entries.empty();++i)queue.one();if(!queue.entries.empty())throw std::logic_error("Focus notification queue did not drain");}
 std::array<std::int32_t,12> snapshot(){std::array<std::int32_t,12> r{index(state.focused),static_cast<std::int32_t>(state.live_update_pending().load()),static_cast<std::int32_t>(queue.entries.size()),static_cast<std::int32_t>(notified)};for(unsigned i=0;i<2;++i)r[4+i]=nodes[i].lifetime_guard()->component!=nullptr;r[6]=static_cast<std::int32_t>(listeners.size());for(unsigned i=0;i<5;++i){r[7+i]=-1;if(i<listeners.size())for(unsigned j=0;j<5;++j)if(listeners[i]==&objects[j])r[7+i]=static_cast<std::int32_t>(j);}return r;}
};
void Listener::global_focus_changed(GuiVisibilityComponent* c){driver->notification(id,c);}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;constexpr Identity expected={"desktop-focus","DESKFC01",21196,112,3263613869u,{0x69,0x7e,0x3b,0x8,0xd7,0xff,0x39,0x1e,0xe3,0xb3,0x5,0xba,0x12,0x97,0x99,0xda,0xca,0xdb,0x65,0x6b,0xc9,0x79,0x7e,0x2f,0x8a,0x86,0xe,0x25,0xf6,0xd3,0x90,0xfe}};auto reader=Reader::file(argv[1],expected);unsigned differences=0,notifications=0;
 for(unsigned i=0;i<expected.count;++i){std::array<std::int32_t,7> config;for(auto& v:config)v=reader.i32();Driver driver(config);for(unsigned phase=0;phase<2;++phase){std::array<std::int32_t,12> native;for(auto& v:native)v=reader.i32();const auto count=reader.u32();std::vector<Event> events(count);for(auto& e:events)for(auto& v:e)v=reader.i32();driver.events.clear();if(phase==0)driver.first();else driver.drain();const auto actual=driver.snapshot();if(actual!=native||driver.events!=events){if(differences++<4){std::cerr<<"Desktop focus mismatch "<<i<<':'<<phase<<" state="<<(actual!=native)<<" events="<<driver.events.size()<<'/'<<events.size()<<'\n';for(unsigned j=0;j<12;++j)if(actual[j]!=native[j])std::cerr<<" state "<<j<<' '<<actual[j]<<'/'<<native[j]<<'\n';}}notifications+=count;}}
 reader.end();if(notifications!=200)return 2;std::cout<<"{\"cases\":"<<expected.count<<",\"notifications\":"<<notifications<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
