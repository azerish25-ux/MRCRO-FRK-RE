#include "mfrk/ui.hpp"
#include <jni.h>
namespace {
jfloatArray result(JNIEnv* env,const std::array<float,4>& values){auto out=env->NewFloatArray(4);if(out)env->SetFloatArrayRegion(out,0,4,values.data());return out;}
}
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_mfrk_renderercomparison_NativeGeometry_center(JNIEnv* env,jclass,jint width,jint height,jfloat radius,jfloat center){return result(env,mfrk::ui::interpolated_center_bounds(width,height,radius,radius,center,center,0.f));}
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_mfrk_renderercomparison_NativeGeometry_outline(JNIEnv* env,jclass,jint width,jint height,jfloat angle,jfloat gradient,jfloat radius){return result(env,mfrk::ui::outline_gradient_points(width,height,angle,angle,gradient,radius));}
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_mfrk_renderercomparison_NativeGeometry_marker(JNIEnv* env,jclass,jint width,jint height,jfloat position,jfloat angle,jfloat radius,jfloat start,jfloat markerWidth){return result(env,mfrk::ui::center_marker_points(width,height,position,false,angle,angle,radius,start,markerWidth));}
