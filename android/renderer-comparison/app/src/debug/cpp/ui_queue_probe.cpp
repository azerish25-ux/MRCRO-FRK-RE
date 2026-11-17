#include "ui_message_queue.hpp"
#include <jni.h>
#include <array>
#include <atomic>
#include <exception>
#include <map>
#include <mutex>
#include <thread>
#include <unistd.h>
namespace {
using namespace mfrk;
struct Counters {
 std::atomic<unsigned> count{0},wrong_thread{0},depth{0},maximum_depth{0};
 std::atomic<unsigned> tag{0},size{0};
 std::array<std::atomic<std::uint32_t>,4> values{};
};
struct Recipient:ui::GuiControllerRecipient {
 std::shared_ptr<Counters> counters;bool nested;
 std::weak_ptr<ui::GuiMessageListener> listener;
 Recipient(std::shared_ptr<Counters> c,bool repeat):counters(std::move(c)),nested(repeat){}
 void write(std::uint32_t origin,std::uint32_t id,preset_bank::Value& value)override{
  if(origin!=73||id!=13)throw std::logic_error("Queue observer address");
  if(value.tag()!=1){
   if(value.tag()!=2||value.bytes().size()!=24)throw std::logic_error("Queue observer data size");
   for(unsigned i=0;i<24;++i)if(value.bytes()[i]!=static_cast<std::uint8_t>(37+i*17))throw std::logic_error("Queue observer data bytes");
  }
  const auto depth=counters->depth.fetch_add(1)+1;
  counters->maximum_depth.store(std::max(depth,counters->maximum_depth.load()));
  if(static_cast<std::uint32_t>(gettid())!=android_ui::main_thread_id())++counters->wrong_thread;
  std::uint32_t bits;std::memcpy(&bits,value.bytes().data(),4);counters->tag=value.tag();counters->size=value.bytes().size();
  const auto at=counters->count.fetch_add(1);if(at<counters->values.size())counters->values[at]=bits;
  if(nested&&at==0)android_ui::post_write(listener,std::make_unique<ui::GuiWriteMessage>(ui::GuiWriteMessage{73,13,preset_bank::Value::scalar(0x3f000000)}));
  --counters->depth;
 }
 void other_message(std::uint32_t,std::uint32_t,std::uint32_t,preset_bank::Value&)override{throw std::logic_error("Queue observer message kind");}
};
struct Probe {
 std::shared_ptr<Counters> counters=std::make_shared<Counters>();
 std::shared_ptr<Recipient> owner;
 std::shared_ptr<ui::GuiMessageListener> listener;
 explicit Probe(bool nested):owner(std::make_shared<Recipient>(counters,nested)),listener(std::make_shared<ui::GuiControllerMessageListener>(owner)){owner->listener=listener;}
};
std::mutex mutex;std::map<std::uint64_t,std::shared_ptr<Probe>> probes;std::uint64_t next=1;
std::shared_ptr<Probe> find(jlong handle){std::lock_guard lock(mutex);const auto at=probes.find(static_cast<std::uint64_t>(handle));if(at==probes.end())throw std::invalid_argument("Live queue observer required");return at->second;}
void main_thread(){if(static_cast<std::uint32_t>(gettid())!=android_ui::main_thread_id())throw std::logic_error("Observer lifecycle requires UI thread");}
void fail(JNIEnv* env,const char* message){if(auto type=env->FindClass("java/lang/IllegalStateException"))env->ThrowNew(type,message);}
}
extern "C" JNIEXPORT jlong JNICALL Java_com_mfrk_renderercomparison_NativeUiQueueProbe_create(JNIEnv* env,jclass,jboolean nested){
 try{main_thread();auto value=std::make_shared<Probe>(nested==JNI_TRUE);std::lock_guard lock(mutex);const auto id=next++;probes.emplace(id,std::move(value));return static_cast<jlong>(id);}catch(const std::exception& error){fail(env,error.what());return 0;}
}
extern "C" JNIEXPORT jlong JNICALL Java_com_mfrk_renderercomparison_NativeUiQueueProbe_postFromWorker(JNIEnv* env,jclass,jlong handle,jint count,jboolean heap_data){
 try{
  if(count<1||count>3)throw std::invalid_argument("Queue observer count");
  const auto probe=find(handle);const auto listener=std::weak_ptr<ui::GuiMessageListener>(probe->listener);std::exception_ptr failure;std::uint64_t last=0;
  std::thread worker([&]{try{
   const std::array<std::uint32_t,3> bits{0x3e000000,0x3e800000,0x3f000000};
   for(int i=0;i<count;++i){
    auto value=preset_bank::Value::scalar(bits[i]);
    if(heap_data==JNI_TRUE){std::array<std::uint8_t,24> raw;for(unsigned j=0;j<raw.size();++j)raw[j]=static_cast<std::uint8_t>(37+j*17);value=preset_bank::Value::data(raw);}
    last=android_ui::post_write(listener,std::make_unique<ui::GuiWriteMessage>(ui::GuiWriteMessage{73,13,std::move(value)}));
   }
  }catch(...){failure=std::current_exception();}});
  worker.join();if(failure)std::rethrow_exception(failure);return static_cast<jlong>(last);
 }catch(const std::exception& error){fail(env,error.what());return 0;}
}
extern "C" JNIEXPORT void JNICALL Java_com_mfrk_renderercomparison_NativeUiQueueProbe_expire(JNIEnv* env,jclass,jlong handle,jboolean listener){
 try{main_thread();auto probe=find(handle);if(listener==JNI_TRUE)probe->listener.reset();else probe->owner.reset();}catch(const std::exception& error){fail(env,error.what());}
}
extern "C" JNIEXPORT jboolean JNICALL Java_com_mfrk_renderercomparison_NativeUiQueueProbe_cancel(JNIEnv* env,jclass,jlong handle){
 try{return android_ui::cancel_message(static_cast<std::uint64_t>(handle))?JNI_TRUE:JNI_FALSE;}catch(const std::exception& error){fail(env,error.what());return JNI_FALSE;}
}
extern "C" JNIEXPORT jintArray JNICALL Java_com_mfrk_renderercomparison_NativeUiQueueProbe_snapshot(JNIEnv* env,jclass,jlong handle){
 try{const auto c=find(handle)->counters;std::array<jint,9> values{static_cast<jint>(c->count.load()),static_cast<jint>(c->wrong_thread.load()),static_cast<jint>(c->maximum_depth.load())};for(unsigned i=0;i<4;++i)values[3+i]=static_cast<jint>(c->values[i].load());values[7]=c->tag.load();values[8]=c->size.load();auto out=env->NewIntArray(values.size());if(out)env->SetIntArrayRegion(out,0,values.size(),values.data());return out;}catch(const std::exception& error){fail(env,error.what());return nullptr;}
}
extern "C" JNIEXPORT void JNICALL Java_com_mfrk_renderercomparison_NativeUiQueueProbe_close(JNIEnv* env,jclass,jlong handle){
 try{main_thread();std::shared_ptr<Probe> removed;{std::lock_guard lock(mutex);const auto at=probes.find(static_cast<std::uint64_t>(handle));if(at!=probes.end()){removed=std::move(at->second);probes.erase(at);}}}catch(const std::exception& error){fail(env,error.what());}
}
