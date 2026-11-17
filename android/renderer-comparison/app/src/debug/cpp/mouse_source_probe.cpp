#include "ui_async_update_queue.hpp"
#include "ui_message_queue.hpp"
#include "mfrk/ui_pointer_geometry.hpp"
#include <jni.h>
#include <map>
#include <mutex>
#include <unistd.h>
namespace {
using namespace mfrk;
struct Probe;
struct Peer:ui::GuiPointerPeer {
 Probe* owner;
 explicit Peer(Probe& probe):owner(&probe){}
 ui::GuiVisibilityComponent& root_component()override;
 ui::GuiMousePoint local_to_global(ui::GuiMousePoint p)override{return p;}
 ui::GuiMousePoint global_to_local(ui::GuiMousePoint p)override{return p;}
 bool contains(std::int32_t x,std::int32_t y,bool children)override{if(!children)throw std::logic_error("Peer child containment required");return x>=0&&y>=0&&x<200&&y<160;}
 std::uint32_t identifier()override{return 42;}
 ui::GuiDirtyBounds client_bounds()override{throw std::logic_error("Unexpected peer bounds");}
 void invalidate(const ui::GuiDirtyBounds&)override{throw std::logic_error("Unexpected peer repaint");}
 bool minimized()override{throw std::logic_error("Unexpected peer visibility");}
 void set_visible(bool)override{throw std::logic_error("Unexpected peer visibility");}
 void grab_focus()override{throw std::logic_error("Unexpected peer focus");}
 bool focused()override{throw std::logic_error("Unexpected peer focus");}
};
struct Probe:ui::GuiPointerGeometryBoundary,ui::GuiMouseGeometryBinding {
 ui::GuiMouseSourceState state;
 ui::GuiVisibilityComponent root;
 Peer window{*this};
 std::array<std::unique_ptr<ui::GuiVisibilityComponent>,2> nodes;
 std::unique_ptr<ui::GuiMouseSourceUpdates> updates;
 unsigned mode,moves=0,drags=0,enters=0,exits=0,finds=0,cursors=0,wrong_thread=0,pending_in_event=0;
 std::int64_t event_time=0;
 explicit Probe(unsigned scenario):ui::GuiMouseGeometryBinding(static_cast<ui::GuiPointerGeometryBoundary&>(*this)),mode(scenario){
  for(auto& node:nodes)node=std::make_unique<ui::GuiVisibilityComponent>();
  root.flags=3;root.bounds={0,0,200,160};
  for(auto& node:nodes){node->parent=&root;node->bounds={0,0,100,80};node->flags=2;root.children.push_back(node.get());}
  nodes[1]->bounds={20,20,100,80};nodes[1]->flags=mode==4?2:0;state.peer=&window;
  state.target=nodes[0]->lifetime_guard();state.position={30.f,40.f};state.last_time=mode==5?1500:900;
  const std::array<float,2> start{30.f,40.f};std::memcpy(state.presses[0].data(),start.data(),8);
  if(mode==1||mode==2){state.modifiers=16;state.position=mode==1?ui::GuiMousePoint{32.f,42.f}:ui::GuiMousePoint{34.f,40.f};}
  if(mode==3){root.children.erase(root.children.begin());nodes[0].reset();}
  updates=std::make_unique<ui::GuiMouseSourceUpdates>(state,*this,android_ui::async_update_queue());
 }
 void event(std::int64_t time){if(static_cast<std::uint32_t>(gettid())!=android_ui::main_thread_id())++wrong_thread;if(state.pending_word().load()!=0)++pending_in_event;event_time=time;}
 ui::GuiPointerPeer* peer(ui::GuiVisibilityComponent&)override{return &window;}
 bool valid_peer(ui::GuiPointerPeer* peer)override{++finds;return peer==&window;}
 float component_scale(ui::GuiVisibilityComponent&)override{return 1.f;}
 bool hit_test(ui::GuiVisibilityComponent& node,std::int32_t,std::int32_t)override{return &node!=&root||mode!=3;}
 std::uint32_t global_modifiers()override{throw std::logic_error("Unexpected mouse-up lookup");}
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
 float desktop_scale()override{return 1.f;}
 ui::GuiMousePoint scaled_to_unscaled(ui::GuiMousePoint)override{throw std::logic_error("Unexpected cursor transform");}
 ui::GuiMousePoint unscaled_to_scaled(ui::GuiMousePoint)override{throw std::logic_error("Unexpected cursor transform");}
 void warp_cursor(ui::GuiMousePoint)override{throw std::logic_error("Unexpected cursor warp");}
 std::int64_t current_time_ms()override{return 1000;}
};
ui::GuiVisibilityComponent& Peer::root_component(){return owner->root;}
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
