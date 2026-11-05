#include "audio_output.hpp"
#include <cstddef>
#include <span>

namespace mfrk::android_audio {
AudioOutput::~AudioOutput(){close();}
aaudio_data_callback_result_t AudioOutput::data_callback(AAudioStream*,void* context,void* buffer,int32_t frames) noexcept {
 auto& self=*static_cast<AudioOutput*>(context);
 if(!buffer||frames<0){self.error_.store(AAUDIO_ERROR_ILLEGAL_ARGUMENT,std::memory_order_release);return AAUDIO_CALLBACK_RESULT_STOP;}
 if(!self.adapter_.fill(std::span(static_cast<float*>(buffer),static_cast<std::size_t>(frames)*2))){self.error_.store(AAUDIO_ERROR_INTERNAL,std::memory_order_release);return AAUDIO_CALLBACK_RESULT_STOP;}
 return AAUDIO_CALLBACK_RESULT_CONTINUE;
}
void AudioOutput::error_callback(AAudioStream*,void* context,aaudio_result_t error) noexcept {
 static_cast<AudioOutput*>(context)->error_.store(error,std::memory_order_release);
}
aaudio_result_t AudioOutput::open_and_start() noexcept {
 if(stream_)return AAUDIO_ERROR_INVALID_STATE;
 if(!render_)return AAUDIO_ERROR_NULL;
 adapter_.reset();error_.store(AAUDIO_OK,std::memory_order_release);
 AAudioStreamBuilder* builder=nullptr;
 auto result=AAudio_createStreamBuilder(&builder);if(result!=AAUDIO_OK)return result;
 AAudioStreamBuilder_setDirection(builder,AAUDIO_DIRECTION_OUTPUT);
 AAudioStreamBuilder_setFormat(builder,AAUDIO_FORMAT_PCM_FLOAT);
 AAudioStreamBuilder_setChannelCount(builder,2);
 AAudioStreamBuilder_setSampleRate(builder,48000);
 AAudioStreamBuilder_setPerformanceMode(builder,AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
 AAudioStreamBuilder_setDataCallback(builder,data_callback,this);
 AAudioStreamBuilder_setErrorCallback(builder,error_callback,this);
 result=AAudioStreamBuilder_openStream(builder,&stream_);
 AAudioStreamBuilder_delete(builder);
 if(result!=AAUDIO_OK){stream_=nullptr;return result;}
 if(AAudioStream_getSampleRate(stream_)!=48000){close();return AAUDIO_ERROR_INVALID_RATE;}
 if(AAudioStream_getChannelCount(stream_)!=2||AAudioStream_getFormat(stream_)!=AAUDIO_FORMAT_PCM_FLOAT){close();return AAUDIO_ERROR_INVALID_FORMAT;}
 result=AAudioStream_requestStart(stream_);
 if(result!=AAUDIO_OK)close();
 return result;
}
void AudioOutput::close() noexcept {
 if(!stream_)return;
 AAudioStream_requestStop(stream_);
 AAudioStream_close(stream_);
 stream_=nullptr;
}
}
