#pragma once
#include "mfrk/audio_block_adapter.hpp"
#include <aaudio/AAudio.h>
#include <atomic>

namespace mfrk::android_audio {
// Control methods are serialized by the owner on one non-audio thread. The
// render callback and its context must remain valid until close() returns.
// This 48 kHz profile cannot start without a real graph callback.
class AudioOutput {
 audio::BlockAdapter adapter_;
 audio::RenderBlock render_;
 AAudioStream* stream_=nullptr;
 std::atomic<aaudio_result_t> error_{AAUDIO_OK};
 static aaudio_data_callback_result_t data_callback(AAudioStream*,void*,void*,int32_t) noexcept;
 static void error_callback(AAudioStream*,void*,aaudio_result_t) noexcept;
public:
 AudioOutput(audio::RenderBlock render,void* context) noexcept:adapter_(render,context),render_(render){}
 ~AudioOutput();
 AudioOutput(const AudioOutput&)=delete;AudioOutput& operator=(const AudioOutput&)=delete;
 aaudio_result_t open_and_start() noexcept;
 void close() noexcept;
 // The owner polls this off the callback thread and decides when to reopen.
 aaudio_result_t error() const noexcept {return error_.load(std::memory_order_acquire);}
};
}
