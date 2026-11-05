#include "mfrk/midi_ingress.hpp"
#include <jni.h>
#include <new>
#include <span>

namespace {
mfrk::midi::Ingress* ingress(jlong handle) noexcept {
 return reinterpret_cast<mfrk::midi::Ingress*>(static_cast<std::uintptr_t>(handle));
}
}
extern "C" JNIEXPORT jlong JNICALL
Java_com_mfrk_midihost_NativeMidiBridge_nativeCreate(JNIEnv*,jclass) {
 auto* value=new(std::nothrow) mfrk::midi::Ingress;
 return static_cast<jlong>(reinterpret_cast<std::uintptr_t>(value));
}
extern "C" JNIEXPORT void JNICALL
Java_com_mfrk_midihost_NativeMidiBridge_nativeDestroy(JNIEnv*,jclass,jlong handle) {delete ingress(handle);}
extern "C" JNIEXPORT jboolean JNICALL
Java_com_mfrk_midihost_NativeMidiBridge_nativeFeed(JNIEnv* env,jclass,jlong handle,jint device,jint port,jbyteArray data,jlong timestamp) {
 auto* value=ingress(handle);if(!value||!data)return JNI_FALSE;
 const auto length=env->GetArrayLength(data);
 jbyte* bytes=env->GetByteArrayElements(data,nullptr);if(!bytes)return JNI_FALSE;
 const bool ok=value->feed(device,port,std::span(reinterpret_cast<const std::uint8_t*>(bytes),static_cast<std::size_t>(length)),timestamp);
 env->ReleaseByteArrayElements(data,bytes,JNI_ABORT);
 return ok?JNI_TRUE:JNI_FALSE;
}
extern "C" JNIEXPORT jboolean JNICALL
Java_com_mfrk_midihost_NativeMidiBridge_nativeResetPort(JNIEnv*,jclass,jlong handle,jint device,jint port) {
 auto* value=ingress(handle);return value&&value->reset_port(device,port)?JNI_TRUE:JNI_FALSE;
}
