#include "controller_attachment.hpp"
#include <jni.h>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
namespace {
struct Entry {
 std::shared_ptr<mfrk::androidui::ControllerAttachment> owner;
 mfrk::ui::SliderDragEvent last{};
 bool active=false,busy=false,closing=false;
};
std::mutex entries_mutex;
std::unordered_map<std::uint64_t,std::shared_ptr<Entry>> entries;
std::uint64_t next_id=1;
void fail(JNIEnv* env,const char* message){const auto type=env->FindClass("java/lang/IllegalStateException");if(type)env->ThrowNew(type,message);}
void finish_dispatch(const std::shared_ptr<Entry>& entry){
 bool finish=false;
 {std::lock_guard lock(entries_mutex);entry->busy=false;if(entry->closing&&entry->active){entry->active=false;finish=true;}}
 if(finish){auto release=entry->last;release.modifiers&=~112u;entry->owner->controller().up(release);}
}
void detach(std::uint64_t id){
 std::shared_ptr<Entry> entry;bool finish=false;
 {std::lock_guard lock(entries_mutex);const auto found=entries.find(id);if(found==entries.end())return;entry=found->second;entries.erase(found);entry->closing=true;if(!entry->busy&&entry->active){entry->active=false;finish=true;}}
 if(finish){auto release=entry->last;release.modifiers&=~112u;entry->owner->controller().up(release);}
}
}
namespace mfrk::androidui {
std::uint64_t attach_controller(std::shared_ptr<ControllerAttachment> owner){
 if(!owner)throw std::invalid_argument("Controller owner required");
 auto entry=std::make_shared<Entry>();entry->owner=std::move(owner);
 std::lock_guard lock(entries_mutex);if(next_id>static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))throw std::overflow_error("Controller handle exhausted");
 const auto id=next_id++;entries.emplace(id,std::move(entry));return id;
}
void detach_controller(std::uint64_t id){detach(id);}
std::shared_ptr<ControllerAttachment> controller_attachment(std::uint64_t id){
 std::lock_guard lock(entries_mutex);const auto found=entries.find(id);return found==entries.end()?nullptr:found->second->owner;
}
}
extern "C" JNIEXPORT jboolean JNICALL
Java_com_mfrk_renderercomparison_NativeSliderController_isAttachedNative(JNIEnv*,jclass,jlong handle){
 std::lock_guard lock(entries_mutex);return handle>0&&entries.contains(static_cast<std::uint64_t>(handle))?JNI_TRUE:JNI_FALSE;
}
extern "C" JNIEXPORT void JNICALL
Java_com_mfrk_renderercomparison_NativeSliderController_dispatchNative(JNIEnv* env,jclass,jlong handle,jint action,jint x,jint y,jint modifiers){
 std::shared_ptr<Entry> entry;
 try {
  if(action<0||action>3)throw std::invalid_argument("Pointer action");
  {std::lock_guard lock(entries_mutex);const auto found=entries.find(static_cast<std::uint64_t>(handle));if(handle<=0||found==entries.end())throw std::invalid_argument("Detached controller");entry=found->second;
   if(entry->busy)throw std::logic_error("Recursive controller input");
   if(action==0&&entry->active)throw std::logic_error("Controller already captured");
   if(action==1&&!entry->active)throw std::logic_error("Controller has no captured pointer");
   if(action>=2&&!entry->active)return;
   entry->busy=true;entry->last={x,y,static_cast<std::uint32_t>(modifiers)};entry->active=action<2;
  }
  try {if(action==0)entry->owner->controller().down(entry->last);else if(action==1)entry->owner->controller().drag(entry->last);else entry->owner->controller().up(entry->last);}
  catch(...){finish_dispatch(entry);throw;}
  finish_dispatch(entry);
 }catch(const std::exception& error){fail(env,error.what());}catch(...){fail(env,"Controller input failed");}
}
extern "C" JNIEXPORT void JNICALL
Java_com_mfrk_renderercomparison_NativeSliderController_detachNative(JNIEnv* env,jclass,jlong handle){
 try {if(handle>0)detach(static_cast<std::uint64_t>(handle));}catch(const std::exception& error){fail(env,error.what());}catch(...){fail(env,"Controller detach failed");}
}
