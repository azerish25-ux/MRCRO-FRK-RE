#include "mfrk/voice_audio.hpp"
#include <bit>
namespace mfrk::voice_audio {
namespace { constexpr float f(std::uint32_t bits) noexcept{return std::bit_cast<float>(bits);} }
VoiceAudio::Interpolator32::Interpolator32() noexcept:current(f(0x2b8cbcccu)) {}
VoiceAudio::VoiceAudio(RawTag) noexcept {}
void VoiceAudio::Interpolator32::set_target(float x)noexcept{if(x!=target){target=x;cursor=0;increment=(x-current)*.03125f;}}

void VoiceAudio::Interpolator32::render(std::span<float,32> output)noexcept{
  if(cursor>30){for(auto& x:output)x=target;current=output[31];return;}
  for(unsigned i=0;i<32;++i)scaled[i]=increment*static_cast<float>(i+1);
  for(unsigned i=0;i<32;++i)output[i]=current+scaled[i];
  cursor+=32;current=output[31];
 }

void VoiceAudio::clip_fm(std::span<float,32> output)const noexcept{
  float cap=signed_fm_*10.0f;if(cap>=1.0f)cap=1.0f;
  const float cutoff=static_cast<float>(cutoff_)*f(0x37800200u);
  float minimum=cutoff*-.39215687f,maximum=(.95f-cutoff)*.44247788f;
  if(maximum<0.0f)maximum=maximum*cap;
  if(minimum>0.0f)minimum=minimum*cap;
  // Native MAXPS(signal,min), then MINPS(max,signal): second operand wins ties.
  for(auto& x:output){x=x>minimum?x:minimum;x=maximum<x?maximum:x;}
 }

VoiceAudio VoiceAudio::admitted_zero_owner_arena()noexcept{return VoiceAudio(RawTag{});}

bool VoiceAudio::prepare(const Controls&in)noexcept{
  if(in.selected_lane>1)return false;
  para_=in.para;pre_level_=in.pre_level;lane_=in.selected_lane;cutoff_=in.cutoff_word;
  if(in.volume_dirty){volume_=(static_cast<float>(in.volume_word)*f(0x37800200u))*.5f;level_.set_target(volume_);}
  if(in.fm_dirty){signed_fm_=(static_cast<float>(in.fm_word)*f(0x37800200u))*.5f-.25f;float x=signed_fm_;if(x>=0.0f)x=-x;fm_.set_target(x);}
  const bool position_dirty=in.mixer_dirty[0]!=0,a_dirty=in.mixer_dirty[1]!=0,b_dirty=in.mixer_dirty[2]!=0,mode_dirty=in.mixer_dirty[3]!=0;
  const bool first=mode_dirty||a_dirty||(position_dirty&&fx_);
  const bool second=mode_dirty||position_dirty||b_dirty;
  if(position_dirty)position_=(static_cast<float>(in.mixer_words[0])*f(0x37800200u))*.5f;
  if(a_dirty)vca_a_=static_cast<float>(in.mixer_words[1])*f(0x37800200u);
  if(b_dirty)vca_b_=static_cast<float>(in.mixer_words[2])*f(0x37800200u);
  if(mode_dirty)fx_=in.mixer_words[3]>0x7ffd;
  if(first){factor_a_=fx_?(.5f-position_)*2.0f:1.0f;const float target=static_cast<float>((static_cast<double>(factor_a_)*.5)*static_cast<double>(vca_a_));mix_a_.set_target(target);}
  if(second){factor_b_=fx_?position_*2.0f:position_;const float target=static_cast<float>((static_cast<double>(factor_b_)*.5)*static_cast<double>(vca_b_));mix_b_.set_target(target);}
  return true;
 }

bool VoiceAudio::render(Models models,const void* external,std::span<float,32> main,std::span<float,32> modulation)noexcept{
  if(!models.render)return false;
  models.render(models.context,0,external,a_);level_.render(level_buffer_);fm_.render(fm_buffer_);
  const bool first_fm=para_?lane_==0:signed_fm_<0.0f;
  const bool second_fm=para_?lane_==1:signed_fm_>=0.0f;
  if(first_fm){for(unsigned i=0;i<32;++i)modulation[i]=fm_buffer_[i]*a_[i];clip_fm(modulation);}
  if(pre_level_)for(unsigned i=0;i<32;++i)a_[i]=level_buffer_[i]*a_[i];
  input_b_=a_;models.render(models.context,1,input_b_.data(),b_);
  if(!pre_level_)for(unsigned i=0;i<32;++i)a_[i]=level_buffer_[i]*a_[i];
  if(second_fm){for(unsigned i=0;i<32;++i)modulation[i]=fm_buffer_[i]*b_[i];clip_fm(modulation);}
  mix_a_.render(mix_a_buffer_);mix_b_.render(mix_b_buffer_);
  for(unsigned i=0;i<32;++i){a_[i]=mix_a_buffer_[i]*a_[i];b_[i]=mix_b_buffer_[i]*b_[i];}
  for(unsigned i=0;i<32;++i)main[i]=a_[i];
  for(unsigned i=0;i<32;++i)main[i]=b_[i]+main[i];
  return true;
 }
}
