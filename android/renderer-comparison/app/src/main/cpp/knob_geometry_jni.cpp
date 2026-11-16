#include "mfrk/ui.hpp"
#include <jni.h>
#include <bit>
#include <cmath>
namespace {
jfloatArray result(JNIEnv* env,const std::array<float,4>& values){auto out=env->NewFloatArray(4);if(out)env->SetFloatArrayRegion(out,0,4,values.data());return out;}
jlong packed(float value,bool first,bool second=false){return static_cast<jlong>(std::uint64_t(std::bit_cast<std::uint32_t>(value))|(std::uint64_t(first)<<32)|(std::uint64_t(second)<<33));}
jlong invalid(JNIEnv* env,const char* message){const auto type=env->FindClass("java/lang/IllegalArgumentException");if(type)env->ThrowNew(type,message);return 0;}
bool read_zones(JNIEnv* env,jfloatArray input,std::array<mfrk::ui::MagneticZone,128>& zones,std::size_t& count){
 if(!input){invalid(env,"Magnetic zones required");return false;}
 const auto size=env->GetArrayLength(input);if(size<0||size>640||size%5){invalid(env,"Magnetic zone record count");return false;}
 std::array<jfloat,640> raw{};if(size)env->GetFloatArrayRegion(input,0,size,raw.data());if(env->ExceptionCheck())return false;
 count=static_cast<std::size_t>(size/5);
 for(std::size_t i=0;i<count;++i){const auto* p=raw.data()+i*5;
  for(unsigned j=0;j<4;++j)if(!std::isfinite(p[j])||p[j]<0.f||p[j]>1.f){invalid(env,"Normalized magnetic zone endpoints required");return false;}
  if(p[0]>p[1]||p[2]>p[3]||(p[4]!=0.f&&p[4]!=1.f)||(i&&(p[0]<zones[i-1].position_from||p[2]<zones[i-1].value_from))){invalid(env,"Ordered magnetic zone records required");return false;}
  zones[i]={p[0],p[1],p[2],p[3],p[4]!=0.f};
 }
 return true;
}
}
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_mfrk_renderercomparison_NativeGeometry_center(JNIEnv* env,jclass,jint width,jint height,jfloat radius,jfloat center){return result(env,mfrk::ui::interpolated_center_bounds(width,height,radius,radius,center,center,0.f));}
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_mfrk_renderercomparison_NativeGeometry_outline(JNIEnv* env,jclass,jint width,jint height,jfloat angle,jfloat gradient,jfloat radius){return result(env,mfrk::ui::outline_gradient_points(width,height,angle,angle,gradient,radius));}
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_mfrk_renderercomparison_NativeGeometry_marker(JNIEnv* env,jclass,jint width,jint height,jfloat position,jfloat angle,jfloat radius,jfloat start,jfloat markerWidth){return result(env,mfrk::ui::center_marker_points(width,height,position,false,angle,angle,radius,start,markerWidth));}
extern "C" JNIEXPORT jlong JNICALL
Java_com_mfrk_renderercomparison_NativeMagnetism_phase(JNIEnv* env,jclass,jint phase,jboolean matches,jboolean reverse,jfloat current,jfloat held,jboolean flag20,jboolean flag21){
 if(matches&&(!std::isfinite(current)||!std::isfinite(held)))return invalid(env,"Finite magnetic phase values required");
 mfrk::ui::MagneticGesture state{held,flag20!=JNI_FALSE,flag21!=JNI_FALSE};
 mfrk::ui::magnetic_phase(state,static_cast<std::uint32_t>(phase),matches!=JNI_FALSE,current,reverse!=JNI_FALSE);
 return packed(state.held_value,state.flag20,state.flag21);
}
extern "C" JNIEXPORT jlong JNICALL
Java_com_mfrk_renderercomparison_NativeMagnetism_delta(JNIEnv* env,jclass,jfloatArray input,jfloat value,jfloat change,jboolean flag20,jboolean flag21,jboolean wrap,jboolean matches){
 if(!matches)return packed(-1.f,false);
 if(!std::isfinite(value)||!std::isfinite(change))return invalid(env,"Finite magnetic delta values required");
 std::array<mfrk::ui::MagneticZone,128> zones{};std::size_t count=0;if(!read_zones(env,input,zones,count))return 0;
 if(wrap&&(count==0||zones[count-1].position_to<=0.f))return invalid(env,"Positive magnetic wrap period required");
 const auto mapped=mfrk::ui::magnetic_delta(std::span(zones).first(count),value,change,flag20!=JNI_FALSE,flag21!=JNI_FALSE,wrap!=JNI_FALSE);
 if(!std::isfinite(mapped.value))return invalid(env,"Magnetic mapping outside finite profile");
 return packed(mapped.value,mapped.skip);
}
extern "C" JNIEXPORT jlong JNICALL
Java_com_mfrk_renderercomparison_NativeMagnetism_wheel(JNIEnv* env,jclass,jfloatArray input,jfloat current,jfloat vertical,jboolean reversed,jboolean inertial,jboolean reverse){
 if(inertial)return packed(0.f,false,false);
 if(!std::isfinite(vertical))return invalid(env,"Finite wheel delta required");
 if(vertical==0.f)return packed(0.f,false,false);
 if(!std::isfinite(current))return invalid(env,"Finite wheel parameter required");
 std::array<mfrk::ui::MagneticZone,128> zones{};std::size_t count=0;if(!read_zones(env,input,zones,count))return 0;
 const auto mapped=mfrk::ui::magnetic_wheel(std::span(zones).first(count),current,vertical,reversed!=JNI_FALSE,false,reverse!=JNI_FALSE);
 return packed(mapped.target,mapped.handled,mapped.has_target);
}
