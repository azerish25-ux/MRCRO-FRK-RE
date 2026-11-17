#include "ui_message_queue.hpp"
#include <jni.h>
#include <map>
#include <mutex>
#include <unistd.h>
namespace mfrk::android_ui {
namespace {
struct Runtime {
 std::mutex mutex;
 JavaVM* vm=nullptr;jclass queue=nullptr;jmethodID enqueue=nullptr;
 std::uint32_t thread=0;std::uint64_t next=1;
 std::map<std::uint64_t,std::unique_ptr<ui::GuiMessageEnvelope>> pending;
};
Runtime& runtime(){static Runtime value;return value;}
struct Environment {
 JavaVM* vm;JNIEnv* env=nullptr;bool attached=false;
 explicit Environment(JavaVM* machine):vm(machine){
  const auto result=vm->GetEnv(reinterpret_cast<void**>(&env),JNI_VERSION_1_6);
  if(result==JNI_EDETACHED){
   if(vm->AttachCurrentThread(&env,nullptr)!=JNI_OK)throw std::runtime_error("Attach UI posting thread");
   attached=true;
  }else if(result!=JNI_OK)throw std::runtime_error("UI posting JNI environment");
 }
 ~Environment(){if(attached)vm->DetachCurrentThread();}
};
void fail(JNIEnv* env,const char* message){
 if(env->ExceptionCheck())env->ExceptionClear();
 if(auto type=env->FindClass("java/lang/IllegalStateException"))env->ThrowNew(type,message);
}
}
std::uint32_t main_thread_id(){
 auto& state=runtime();std::lock_guard lock(state.mutex);
 if(!state.vm)throw std::logic_error("Initialize Android UI messages first");
 return state.thread;
}
bool cancel_message(std::uint64_t handle){
 std::unique_ptr<ui::GuiMessageEnvelope> removed;
 {auto& state=runtime();std::lock_guard lock(state.mutex);const auto at=state.pending.find(handle);if(at==state.pending.end())return false;removed=std::move(at->second);state.pending.erase(at);}
 return true;
}
std::uint64_t post_message(std::weak_ptr<ui::GuiMessageListener> listener,std::unique_ptr<ui::GuiUiMessage> message){
 auto envelope=std::make_unique<ui::GuiMessageEnvelope>(std::move(listener),std::move(message));
 JavaVM* vm;jclass queue;jmethodID enqueue;std::uint64_t handle;
 {auto& state=runtime();std::lock_guard lock(state.mutex);
  if(!state.vm)throw std::logic_error("Initialize Android UI messages first");
  if(state.next>static_cast<std::uint64_t>(std::numeric_limits<jlong>::max()))throw std::overflow_error("UI message handles exhausted");
  vm=state.vm;queue=state.queue;enqueue=state.enqueue;handle=state.next++;
  state.pending.emplace(handle,std::move(envelope));
 }
 try{
  Environment environment(vm);const auto accepted=environment.env->CallStaticBooleanMethod(queue,enqueue,static_cast<jlong>(handle));
  if(environment.env->ExceptionCheck()){environment.env->ExceptionClear();throw std::runtime_error("Android UI message post failed");}
  if(accepted!=JNI_TRUE)throw std::runtime_error("Android main loop rejected message");
 }catch(...){cancel_message(handle);throw;}
 return handle;
}
std::uint64_t post_write(std::weak_ptr<ui::GuiMessageListener> listener,std::unique_ptr<ui::GuiWriteMessage> message){
 if(!message)throw std::invalid_argument("Owned write message required");
 return post_message(std::move(listener),std::make_unique<ui::GuiControllerMessage>(0,std::move(*message)));
}
}
extern "C" JNIEXPORT void JNICALL Java_com_mfrk_uihost_UiMessageQueue_nativeInitialize(JNIEnv* env,jclass type){
 using namespace mfrk::android_ui;
 try{
  JavaVM* vm=nullptr;if(env->GetJavaVM(&vm)!=JNI_OK)throw std::runtime_error("Android UI Java VM");
  auto method=env->GetStaticMethodID(type,"enqueue","(J)Z");if(!method)throw std::runtime_error("Android UI enqueue method");
  auto& state=runtime();std::lock_guard lock(state.mutex);
  if(state.vm){if(!env->IsSameObject(state.queue,type))throw std::logic_error("UI dispatcher class changed");return;}
  auto reference=static_cast<jclass>(env->NewGlobalRef(type));if(!reference)throw std::bad_alloc();
  // This process-class reference deliberately lives with the process queue.
  // It holds no Activity, View or scene instance.
  state.queue=reference;state.enqueue=method;state.thread=static_cast<std::uint32_t>(gettid());state.vm=vm;
 }catch(const std::exception& error){fail(env,error.what());}
}
extern "C" JNIEXPORT void JNICALL Java_com_mfrk_uihost_UiMessageQueue_nativeDeliver(JNIEnv* env,jclass,jlong handle){
 using namespace mfrk::android_ui;
 try{
  if(static_cast<std::uint32_t>(gettid())!=main_thread_id())throw std::logic_error("UI message delivery requires the main thread");
  std::unique_ptr<mfrk::ui::GuiMessageEnvelope> message;
  {auto& state=runtime();std::lock_guard lock(state.mutex);const auto at=state.pending.find(static_cast<std::uint64_t>(handle));if(at==state.pending.end())return;message=std::move(at->second);state.pending.erase(at);}
  // Remove before callbacks: nested posts/cancellation cannot deadlock or
  // deliver this handle again. Expired recipients are handled by the envelope.
  message->deliver();
 }catch(const std::exception& error){fail(env,error.what());}
}
