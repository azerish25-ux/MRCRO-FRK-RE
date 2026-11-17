#include "ui_async_update_queue.hpp"
#include "ui_message_queue.hpp"
#include "mfrk/ui_mouse_source.hpp"
#include <jni.h>
#include <map>
#include <mutex>
#include <unistd.h>
namespace {
using namespace mfrk;
struct Probe:ui::GuiMouseInputBoundary {
 ui::GuiMouseSourceState state;
 std::array<std::unique_ptr<ui::GuiVisibilityComponent>,2> nodes;
 std::unique_ptr<ui::GuiMouseSourceUpdates> updates;
 unsigned mode,moves=0,drags=0,enters=0,exits=0,finds=0,cursors=0,wrong_thread=0,pending_in_event=0;
 std::int64_t event_time=0;
 explicit Probe(unsigned scenario):mode(scenario){
  for(auto& node:nodes)node=std::make_unique<ui::GuiVisibilityComponent>();
  state.target=nodes[0]->lifetime_guard();state.position={30.f,40.f};state.last_time=mode==5?1500:900;
  const std::array<float,2> start{30.f,40.f};std::memcpy(state.presses[0].data(),start.data(),8);
  if(mode==1||mode==2){state.modifiers=16;state.position=mode==1?ui::GuiMousePoint{32.f,42.f}:ui::GuiMousePoint{34.f,40.f};}
  if(mode==3)nodes[0].reset();
  updates=std::make_unique<ui::GuiMouseSourceUpdates>(state,*this,android_ui::async_update_queue());
 }
 void event(std::int64_t time){if(static_cast<std::uint32_t>(gettid())!=android_ui::main_thread_id())++wrong_thread;if(state.pending_word().load()!=0)++pending_in_event;event_time=time;}
 ui::GuiVisibilityComponent* find_target(ui::GuiMouseSourceState&,ui::GuiMousePoint)override{++finds;return mode==3?nullptr:nodes[mode==4?1:0].get();}
 ui::GuiMousePoint local_position(ui::GuiVisibilityComponent&,ui::GuiMousePoint p)override{return p;}
 std::uint32_t global_modifiers()override{throw std::logic_error("Unexpected mouse-up lookup");}
 std::uint32_t peer_identifier(ui::GuiVisibilityComponent&)override{throw std::logic_error("Unexpected mouse-down lookup");}
 void increment_mouse_down_count()override{throw std::logic_error("Unexpected mouse-down count");}
 void mouse_enter(ui::GuiVisibilityComponent&,ui::GuiMouseSourceState&,ui::GuiMousePoint,std::int64_t t)override{++enters;event(t);}
 void mouse_exit(ui::GuiVisibilityComponent&,ui::GuiMouseSourceState&,ui::GuiMousePoint,std::int64_t t)override{++exits;event(t);}
 void mouse_move(ui::GuiVisibilityComponent&,ui::GuiMouseSourceState&,ui::GuiMousePoint,std::int64_t t)override{++moves;event(t);}
 void mouse_drag(ui::GuiVisibilityComponent&,ui::GuiMouseSourceState&,ui::GuiMousePoint,std::int64_t t,const ui::GuiMouseExtra&)override{++drags;event(t);}
 void mouse_down(ui::GuiVisibilityComponent&,ui::GuiMouseSourceState&,ui::GuiMousePoint,std::int64_t,const ui::GuiMouseExtra&)override{throw std::logic_error("Unexpected mouse down");}
 void mouse_up(ui::GuiVisibilityComponent&,ui::GuiMouseSourceState&,ui::GuiMousePoint,std::int64_t,std::uint32_t,const ui::GuiMouseExtra&)override{throw std::logic_error("Unexpected mouse up");}
 void update_cursor(ui::GuiMouseSourceState&,bool)override{++cursors;}
 ui::GuiDirtyBounds global_bounds(ui::GuiVisibilityComponent&)override{throw std::logic_error("Unexpected cursor bounds");}
 ui::GuiDirtyBounds display_bounds(const ui::GuiDirtyBounds&)override{throw std::logic_error("Unexpected display bounds");}
 float desktop_scale()override{throw std::logic_error("Unexpected desktop scale");}
 ui::GuiMousePoint scaled_to_unscaled(ui::GuiMousePoint)override{throw std::logic_error("Unexpected cursor transform");}
 ui::GuiMousePoint unscaled_to_scaled(ui::GuiMousePoint)override{throw std::logic_error("Unexpected cursor transform");}
 void warp_cursor(ui::GuiMousePoint)override{throw std::logic_error("Unexpected cursor warp");}
 std::int64_t current_time_ms()override{return 1000;}
};
std::mutex mutex;std::map<std::uint64_t,std::shared_ptr<Probe>> probes;std::uint64_t next=1;
void main_thread(){if(static_cast<std::uint32_t>(gettid())!=android_ui::main_thread_id())throw std::logic_error("Mouse observer requires UI thread");}
void fail(JNIEnv* env,const char* text){if(auto type=env->FindClass("java/lang/IllegalStateException"))env->ThrowNew(type,text);}
std::shared_ptr<Probe> find(jlong id){std::lock_guard lock(mutex);const auto at=probes.find(static_cast<std::uint64_t>(id));if(at==probes.end())throw std::invalid_argument("Live mouse observer required");return at->second;}
}
extern "C" JNIEXPORT jlong JNICALL Java_com_mfrk_renderercomparison_NativeMouseSourceProbe_start(JNIEnv* env,jclass,jint mode){
 try{main_thread();if(mode<0||mode>5)throw std::invalid_argument("Mouse observer mode");auto probe=std::make_shared<Probe>(static_cast<unsigned>(mode));std::uint64_t id;{std::lock_guard lock(mutex);id=next++;probes.emplace(id,probe);}probe->updates->trigger();return static_cast<jlong>(id);}catch(const std::exception& error){fail(env,error.what());return 0;}
}
extern "C" JNIEXPORT jintArray JNICALL Java_com_mfrk_renderercomparison_NativeMouseSourceProbe_snapshot(JNIEnv* env,jclass,jlong id){
 try{main_thread();const auto p=find(id);const std::array<jint,11> values{static_cast<jint>(p->moves),static_cast<jint>(p->drags),static_cast<jint>(p->enters),static_cast<jint>(p->exits),static_cast<jint>(p->finds),static_cast<jint>(p->cursors),static_cast<jint>(p->wrong_thread),static_cast<jint>(p->pending_in_event),static_cast<jint>(p->event_time),p->state.dragged?1:0,static_cast<jint>(p->state.pending_word().load())};auto out=env->NewIntArray(values.size());if(out)env->SetIntArrayRegion(out,0,values.size(),values.data());return out;}catch(const std::exception& error){fail(env,error.what());return nullptr;}
}
extern "C" JNIEXPORT void JNICALL Java_com_mfrk_renderercomparison_NativeMouseSourceProbe_close(JNIEnv* env,jclass,jlong id){
 try{main_thread();std::shared_ptr<Probe> removed;{std::lock_guard lock(mutex);const auto at=probes.find(static_cast<std::uint64_t>(id));if(at!=probes.end()){removed=std::move(at->second);probes.erase(at);}}}catch(const std::exception& error){fail(env,error.what());}
}
