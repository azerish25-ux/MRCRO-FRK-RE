#include "ui_async_update_queue.hpp"
#include "ui_message_queue.hpp"
#include "mfrk/ui_component_focus.hpp"
#include <jni.h>
#include <array>
#include <exception>
#include <map>
#include <mutex>
#include <thread>
#include <unistd.h>
namespace {
using namespace mfrk;
struct Probe;
struct Target:ui::GuiAsyncUpdateTarget {
 Probe* owner;unsigned generation;
 Target(Probe& value,unsigned epoch):owner(&value),generation(epoch){}
 void handle_async_update()override;
};
struct Probe {
 ui::GuiComponentFocusState state;
 std::unique_ptr<Target> retired_target;
 std::unique_ptr<Target> target;
 std::unique_ptr<ui::GuiComponentFocusUpdate> channel;
 unsigned mode,count=0,wrong_thread=0,depth=0,maximum_depth=0,pending_in_callback=0,last_generation=0;
 explicit Probe(unsigned scenario):mode(scenario){bind(0);}
 void bind(unsigned generation){target=std::make_unique<Target>(*this,generation);channel=std::make_unique<ui::GuiComponentFocusUpdate>(state,*target,android_ui::async_update_queue());}
};
void Target::handle_async_update(){
 auto* probe=owner;const auto epoch=generation;
 ++probe->depth;probe->maximum_depth=std::max(probe->maximum_depth,probe->depth);
 if(static_cast<std::uint32_t>(gettid())!=android_ui::main_thread_id())++probe->wrong_thread;
 if(probe->state.live_update_pending().load()!=0)++probe->pending_in_callback;
 const auto before=probe->count++;probe->last_generation=epoch;
 if(probe->mode==3&&before==0)probe->channel->trigger();
 if(probe->mode==6){probe->channel.reset();probe->target.reset();}
 --probe->depth;
}
std::mutex mutex;std::map<std::uint64_t,std::shared_ptr<Probe>> probes;std::uint64_t next=1;
void main_thread(){if(static_cast<std::uint32_t>(gettid())!=android_ui::main_thread_id())throw std::logic_error("Async observer requires UI thread");}
void fail(JNIEnv* env,const char* message){if(auto type=env->FindClass("java/lang/IllegalStateException"))env->ThrowNew(type,message);}
std::shared_ptr<Probe> find(jlong id){std::lock_guard lock(mutex);const auto at=probes.find(static_cast<std::uint64_t>(id));if(at==probes.end())throw std::invalid_argument("Live async observer required");return at->second;}
}
extern "C" JNIEXPORT jlong JNICALL Java_com_mfrk_renderercomparison_NativeAsyncUpdateProbe_start(JNIEnv* env,jclass,jint mode){
 try{
  main_thread();if(mode<0||mode>7)throw std::invalid_argument("Async observer mode");
  auto probe=std::make_shared<Probe>(static_cast<unsigned>(mode));std::uint64_t id;
  {std::lock_guard lock(mutex);id=next++;probes.emplace(id,probe);}
  if(mode==7){
   std::exception_ptr failure;
   std::thread worker([&]{try{for(unsigned i=0;i<3;++i)probe->channel->trigger();}catch(...){failure=std::current_exception();}});
   worker.join();if(failure)std::rethrow_exception(failure);
  }else probe->channel->trigger();
  if(mode==0){probe->channel->trigger();probe->channel->trigger();}
  else if(mode==1)probe->channel->cancel();
  else if(mode==2){probe->channel.reset();probe->target.reset();}
  else if(mode==4){probe->channel->cancel();probe->channel->trigger();}
  else if(mode==5){probe->channel.reset();probe->retired_target=std::move(probe->target);probe->bind(1);probe->channel->trigger();}
  return static_cast<jlong>(id);
 }catch(const std::exception& error){fail(env,error.what());return 0;}
}
extern "C" JNIEXPORT jintArray JNICALL Java_com_mfrk_renderercomparison_NativeAsyncUpdateProbe_snapshot(JNIEnv* env,jclass,jlong id){
 try{
  main_thread();const auto p=find(id);
  const std::array<jint,7> values{static_cast<jint>(p->count),static_cast<jint>(p->wrong_thread),static_cast<jint>(p->maximum_depth),static_cast<jint>(p->pending_in_callback),static_cast<jint>(p->state.live_update_pending().load()),p->state.update_pending_source?1:0,static_cast<jint>(p->last_generation)};
  auto out=env->NewIntArray(values.size());if(out)env->SetIntArrayRegion(out,0,values.size(),values.data());return out;
 }catch(const std::exception& error){fail(env,error.what());return nullptr;}
}
extern "C" JNIEXPORT void JNICALL Java_com_mfrk_renderercomparison_NativeAsyncUpdateProbe_close(JNIEnv* env,jclass,jlong id){
 try{main_thread();std::shared_ptr<Probe> removed;{std::lock_guard lock(mutex);const auto at=probes.find(static_cast<std::uint64_t>(id));if(at!=probes.end()){removed=std::move(at->second);probes.erase(at);}}}catch(const std::exception& error){fail(env,error.what());}
}
