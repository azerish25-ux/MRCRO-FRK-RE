#include "ui_async_update_queue.hpp"
#include "ui_message_queue.hpp"
#include "mfrk/ui_desktop_focus.hpp"
#include <jni.h>
#include <array>
#include <map>
#include <mutex>
#include <unistd.h>
namespace {
using namespace mfrk;
struct Probe;
struct Listener:ui::GuiGlobalFocusListener {Probe* owner=nullptr;unsigned id=0;void global_focus_changed(ui::GuiVisibilityComponent*)override;};
struct Probe {
 ui::GuiComponentFocusState state;
 std::array<std::unique_ptr<ui::GuiVisibilityComponent>,2> nodes;
 std::array<Listener,4> objects;
 std::vector<ui::GuiGlobalFocusListener*> listeners;
 std::unique_ptr<ui::GuiDesktopFocusUpdates> updates;
 unsigned mode,notified=0,wrong_thread=0,depth=0,maximum_depth=0,pending_in_callback=0;
 std::vector<jint> events;
 explicit Probe(unsigned scenario):mode(scenario){
  for(auto& node:nodes)node=std::make_unique<ui::GuiVisibilityComponent>();
  for(unsigned i=0;i<objects.size();++i){objects[i].owner=this;objects[i].id=i;if(i<3)listeners.push_back(&objects[i]);}
  state.focused=nodes[0].get();updates=std::make_unique<ui::GuiDesktopFocusUpdates>(state,listeners,android_ui::async_update_queue());
 }
 void notification(unsigned id,ui::GuiVisibilityComponent* node){
  ++depth;maximum_depth=std::max(depth,maximum_depth);
  if(static_cast<std::uint32_t>(gettid())!=android_ui::main_thread_id())++wrong_thread;
  if(state.live_update_pending().load()!=0)++pending_in_callback;
  jint index=-1;if(node){if(node==nodes[0].get())index=0;else if(node==nodes[1].get())index=1;else throw std::logic_error("Expired focus snapshot passed to listener");}
  events.push_back(static_cast<jint>(id));events.push_back(index);
  if(notified++==0){
   if(mode==1){state.focused=nullptr;nodes[0].reset();}
   else if(mode==2){state.focused=nodes[1].get();updates->trigger();}
   else if(mode==3)listeners.erase(listeners.begin());
   else if(mode==4)listeners.clear();
   else if(mode==5){listeners.push_back(&objects[3]);updates->trigger();}
  }
  --depth;
 }
};
void Listener::global_focus_changed(ui::GuiVisibilityComponent* node){owner->notification(id,node);}
std::mutex mutex;std::map<std::uint64_t,std::shared_ptr<Probe>> probes;std::uint64_t next=1;
void main_thread(){if(static_cast<std::uint32_t>(gettid())!=android_ui::main_thread_id())throw std::logic_error("Desktop focus observer requires UI thread");}
void fail(JNIEnv* env,const char* message){if(auto type=env->FindClass("java/lang/IllegalStateException"))env->ThrowNew(type,message);}
std::shared_ptr<Probe> find(jlong id){std::lock_guard lock(mutex);const auto at=probes.find(static_cast<std::uint64_t>(id));if(at==probes.end())throw std::invalid_argument("Live desktop focus observer required");return at->second;}
}
extern "C" JNIEXPORT jlong JNICALL Java_com_mfrk_renderercomparison_NativeDesktopFocusProbe_start(JNIEnv* env,jclass,jint mode){
 try{main_thread();if(mode<0||mode>5)throw std::invalid_argument("Desktop focus observer mode");auto probe=std::make_shared<Probe>(static_cast<unsigned>(mode));std::uint64_t id;{std::lock_guard lock(mutex);id=next++;probes.emplace(id,probe);}probe->updates->trigger();return static_cast<jlong>(id);}catch(const std::exception& error){fail(env,error.what());return 0;}
}
extern "C" JNIEXPORT jintArray JNICALL Java_com_mfrk_renderercomparison_NativeDesktopFocusProbe_snapshot(JNIEnv* env,jclass,jlong id){
 try{main_thread();const auto p=find(id);std::vector<jint> values{static_cast<jint>(p->notified),static_cast<jint>(p->wrong_thread),static_cast<jint>(p->maximum_depth),static_cast<jint>(p->pending_in_callback),static_cast<jint>(p->state.live_update_pending().load())};values.insert(values.end(),p->events.begin(),p->events.end());auto out=env->NewIntArray(values.size());if(out)env->SetIntArrayRegion(out,0,values.size(),values.data());return out;}catch(const std::exception& error){fail(env,error.what());return nullptr;}
}
extern "C" JNIEXPORT void JNICALL Java_com_mfrk_renderercomparison_NativeDesktopFocusProbe_close(JNIEnv* env,jclass,jlong id){
 try{main_thread();std::shared_ptr<Probe> removed;{std::lock_guard lock(mutex);const auto at=probes.find(static_cast<std::uint64_t>(id));if(at!=probes.end()){removed=std::move(at->second);probes.erase(at);}}}catch(const std::exception& error){fail(env,error.what());}
}
