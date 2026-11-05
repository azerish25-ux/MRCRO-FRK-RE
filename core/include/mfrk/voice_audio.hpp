#pragma once
#include <array>
#include <cstdint>
#include <span>
namespace mfrk::voice_audio {

// Packed source member-field controls and prepared routing flags.
// Upstream pitch/gates/selection/parameter compilation are separate.
struct Controls {
 std::uint16_t volume_word,fm_word,cutoff_word;
 std::uint8_t volume_dirty,fm_dirty;
 bool para,pre_level;
 std::uint8_t selected_lane;
 std::array<std::uint16_t,4> mixer_words;
 std::array<std::uint8_t,4> mixer_dirty;
};
// Render hooks represent actual persistent selected model implementations.
// They must fully write32 finite scaled samples without reentering this actor.
// Side0 receives the caller's opaque external context; side1 receives32float
// generated mono input. The valid model/controls/intermediate domain is caller-owned.
struct Models {void* context;void(*render)(void*,unsigned,const void*,std::span<float,32>)noexcept;};
struct TestAccess;
class VoiceAudio {
 friend struct TestAccess;
 struct Interpolator32 {
 Interpolator32() noexcept;
 float target=0,current,increment=0;
 std::uint32_t cursor=0;
 std::array<float,32> scaled{};
 void set_target(float x)noexcept;
 void render(std::span<float,32> output)noexcept;
};
 float volume_=1,signed_fm_=0;
 bool para_=false,pre_level_=false;
 std::uint8_t lane_=0;
 std::uint16_t cutoff_=0;
 float position_=0,vca_a_=1,vca_b_=1,factor_a_=1,factor_b_=0;
 bool fx_=false;
 Interpolator32 level_,fm_,mix_a_,mix_b_;
 std::array<float,32> a_{},b_{},input_b_{},level_buffer_{},fm_buffer_{},mix_a_buffer_{},mix_b_buffer_{};
 struct RawTag{};
 explicit VoiceAudio(RawTag)noexcept;
 void clip_fm(std::span<float,32> output)const noexcept;
public:
 VoiceAudio(const VoiceAudio&)=delete;VoiceAudio&operator=(const VoiceAudio&)=delete;
 VoiceAudio(VoiceAudio&&)=delete;VoiceAudio&operator=(VoiceAudio&&)=delete;
 static VoiceAudio admitted_zero_owner_arena()noexcept;
 // Fixed metadata profile: volume0..0.5, FM-0.25..0.25, mixerposition0..0.5.
 // These are packed member-field inputs, not frontend controls/type selection.
 bool prepare(const Controls&in)noexcept;
 // main and FM outputs must be disjoint valid32float spans. Model hooks have
 // their own proven domain; no generic model/noise/waveform substitute is used.
 bool render(Models models,const void* external,std::span<float,32> main,std::span<float,32> modulation)noexcept;
};
}
