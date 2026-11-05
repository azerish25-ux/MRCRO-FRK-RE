#pragma once
#include "mfrk/voice_audio.hpp"
#include <bit>
namespace mfrk::voice_audio {
struct TestAccess {
 static auto snapshot(const VoiceAudio&v)noexcept {
  std::array<std::uint32_t,154> r{};unsigned i=0;const auto put=[&](float x){r[i++]=std::bit_cast<std::uint32_t>(x);};
  put(v.volume_);put(v.signed_fm_);put(v.position_);put(v.vca_a_);put(v.vca_b_);put(v.factor_a_);put(v.factor_b_);put(.5f);put(2.f);r[i++]=v.fx_;
  for(const auto* p:{&v.level_,&v.fm_,&v.mix_a_,&v.mix_b_}){put(p->target);put(p->current);r[i++]=p->cursor;put(p->increment);for(auto x:p->scaled)put(x);}return r;
 }
 static auto modes(const VoiceAudio&v)noexcept{return std::array<unsigned,4>{v.para_,v.pre_level_,v.lane_,v.cutoff_};}
};
}
