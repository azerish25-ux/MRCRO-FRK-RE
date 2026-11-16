#include "../../main/cpp/controller_attachment.hpp"
#include "mfrk/ui_slider_curves.hpp"
#include <jni.h>
#include <cmath>
#include <stdexcept>
namespace {
unsigned total_ends=0,total_destroyed=0;
// An explicit observer host, compiled only into the Debug comparison library.
// The production attachment API requires the actual application controller host.
struct Probe final:mfrk::androidui::ControllerAttachment,mfrk::ui::SliderControllerHost {
 std::unique_ptr<mfrk::ui::SliderCurve> curve=mfrk::ui::make_slider_curve({mfrk::ui::SliderCurveKind::linear,{},{}});
 float value;unsigned prepares=0,ends=0,writes=0;
 mfrk::ui::MagneticGesture gesture{-1,false,false};
 mfrk::ui::SliderValueInfo metadata{*curve,0,1,-1,-1,false,false};
 mfrk::ui::SliderPointerConfig pc{false,false,false,false,false,2,false};
 mfrk::ui::SliderPointerState ps{false,0xffffffffu,0};
 mfrk::ui::SliderDragConfig dc{0,.001f,false,false,false};
 mfrk::ui::SliderDragState ds{0,0xffffffffu,0,0,0,false,false};
 mfrk::ui::SliderValueConfig vc{false,0,0,false,false,false};
 mfrk::ui::SliderController source{pc,ps,dc,ds,vc,*this};
 explicit Probe(float v):value(v){}
 ~Probe()override{++total_destroyed;}
 mfrk::ui::SliderController& controller()override{return source;}
 void prepare(std::uint32_t)override{++prepares;}
 mfrk::ui::SliderValueInfo& info(std::uint32_t,bool)override{return metadata;}
 void edit(std::uint32_t,std::uint32_t,bool)override{}
 bool blocked(std::uint32_t)override{return false;}
 float sensitivity(const mfrk::ui::SliderDragEvent& e,std::uint32_t)override{return mfrk::ui::slider_sensitivity({1000,.005f,.001f,0,125,125,0,e.modifiers,true,false,false,false});}
 std::array<std::int32_t,2> global_position(const mfrk::ui::SliderDragEvent& e)override{return {e.x,e.y};}
 std::int32_t item_count(std::uint32_t)override{return 0;}
 std::uint32_t selected_id()override{return 42;}
 void index(std::uint32_t)override{}
 void set(std::uint32_t,float v,bool notify)override{if(!notify)throw std::logic_error("Source notify");value=v;++writes;}
 void mapped(double,double)override{}
 void normalized(double)override{}
 void id_value(std::uint32_t,float)override{}
 bool parent_enabled()override{return true;}
 std::int32_t pointer_mode()override{return 1;}
 void enable_unbounded_pointer()override{throw std::logic_error("Touch cannot warp a cursor");}
 bool phase(std::uint32_t phase,const mfrk::ui::SliderDragEvent&,std::uint32_t& id)override{mfrk::ui::magnetic_phase(gesture,phase,id==42,value,false);return false;}
 float current(std::uint32_t)override{return value;}
 mfrk::ui::MagneticResult map(float v,float delta,std::uint32_t id)override{constexpr std::array<mfrk::ui::MagneticZone,1> zones{{{0,1,0,1,false}}};return mfrk::ui::magnetic_delta(zones,v,delta,gesture.flag20,gesture.flag21,false,id==42);}
 void after_down(const mfrk::ui::SliderDragEvent&)override{}
 void after_drag(const mfrk::ui::SliderDragEvent&)override{}
 void before_up(const mfrk::ui::SliderDragEvent&,bool)override{}
 void end_edit(std::uint32_t)override{++ends;++total_ends;}
 void reset_timer()override{}
 void after_up(const mfrk::ui::SliderDragEvent&)override{}
};
void fail(JNIEnv* env,const char* message){auto type=env->FindClass("java/lang/IllegalArgumentException");if(type)env->ThrowNew(type,message);}
}
extern "C" JNIEXPORT jlong JNICALL
Java_com_mfrk_renderercomparison_NativeGestureProbe_create(JNIEnv* env,jclass,jfloat initial){
 try {if(!std::isfinite(initial)||initial<0||initial>1)throw std::invalid_argument("Normalized probe input");return static_cast<jlong>(mfrk::androidui::attach_controller(std::make_shared<Probe>(initial)));}
 catch(const std::exception& error){fail(env,error.what());return 0;}
}
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_mfrk_renderercomparison_NativeGestureProbe_snapshot(JNIEnv* env,jclass,jlong handle){
 auto probe=std::dynamic_pointer_cast<Probe>(mfrk::androidui::controller_attachment(static_cast<std::uint64_t>(handle)));
 if(!probe){fail(env,"Live probe attachment required");return nullptr;}
 const std::array<jfloat,4> values{probe->value,static_cast<float>(probe->prepares),static_cast<float>(probe->ends),static_cast<float>(probe->writes)};
 auto out=env->NewFloatArray(4);if(out)env->SetFloatArrayRegion(out,0,4,values.data());return out;
}
extern "C" JNIEXPORT jintArray JNICALL
Java_com_mfrk_renderercomparison_NativeGestureProbe_counters(JNIEnv* env,jclass){
 const std::array<jint,2> values{static_cast<jint>(total_ends),static_cast<jint>(total_destroyed)};
 auto out=env->NewIntArray(2);if(out)env->SetIntArrayRegion(out,0,2,values.data());return out;
}
