#include "mfrk/master_output.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
namespace mfrk::master {
namespace { constexpr float f(std::uint32_t bits) noexcept{return std::bit_cast<float>(bits);} }
Limiter::Limiter(DelayMemory& memory) noexcept : memory_(memory),
 attack_(f(0x3d0bc810u)),release_(f(0x38a4e000u)),peak_decay_(f(0x3b086400u)),threshold_(f(0x33d6bf95u)) {}
Master::Master(DelayMemory& memory,RawTag) noexcept : limiter_(memory),current_(f(0x2b8cbcccu)),
 coefficient_(f(0x3be2c480u)),threshold_(f(0x2b8cbcccu)) {}
Master::Master(DelayMemory& memory,ReadyTag) noexcept : Master(memory,RawTag{}) {reset();}
Master Master::admitted_zero_owner_arena(DelayMemory& memory) noexcept {return Master(memory,RawTag{});}
Master Master::admitted_ready_owner(DelayMemory& memory) noexcept {return Master(memory,ReadyTag{});}
void Limiter::reset() noexcept{
    peak_.fill(0.0f); gain_db_.fill(0.0f); valid_.fill(0); write_.fill(0);
  }

void Limiter::process(const StereoBlock& input,StereoBlock& output) noexcept{
    for(unsigned channel=0;channel<2;++channel) {
      for(unsigned sample=0;sample<16;++sample) {
        const float x=input[channel][sample];
        float level=std::fabs(x);
        if(level<peak_[channel]) level=(level-peak_[channel])*peak_decay_+peak_[channel];
        peak_[channel]=level;
        float desired_db=0.0f;
        if(level>=threshold_) {
          const auto bits=std::bit_cast<std::uint32_t>(level+f(0x2edbe6ffu));
          const auto exponent=bits&0x7f800000u;
          const float mantissa=f((bits-exponent)|0x3f800000u);
          float polynomial=mantissa*f(0x3e27fac6u)-f(0x3f8ca79eu);
          polynomial=polynomial*mantissa+f(0x40497db7u);
          const float power=static_cast<float>(static_cast<int>(exponent>>23)-127);
          polynomial=polynomial*mantissa+(power-f(0x400da994u));
          const float db=polynomial*f(0x40c0a8c2u);
          if(db> -1.0f) desired_db=-1.0f-db;
        }
        const float coefficient=std::fabs(desired_db)<=std::fabs(gain_db_[channel])?release_:attack_;
        gain_db_[channel]=coefficient*(desired_db-gain_db_[channel])+gain_db_[channel];
        const float q=gain_db_[channel]*f(0x3e2a152du);
        float gain=0.0f;
        if(q>= -126.0f) {
          int integral=static_cast<int>(q);
          if(q<0.0f) --integral; // Literal source branch, including exact negative integers.
          const float fraction=q-static_cast<float>(integral);
          float polynomial=fraction*f(0x3da2b23fu)+f(0x3e68de81u);
          polynomial=polynomial*fraction+f(0x3f317218u);
          polynomial=polynomial*fraction+1.0f;
          gain=polynomial*f(static_cast<std::uint32_t>(integral+127)*0x800000u);
        }
        const unsigned read=(write_[channel]+1u)%145u; // (write-144) modulo145.
        const float delayed=read<valid_[channel]?memory_.samples[channel][read]:0.0f;
        output[channel][sample]=delayed*gain;
        memory_.samples[channel][write_[channel]]=x;
        if(valid_[channel]==write_[channel]) ++valid_[channel];
        write_[channel]=(write_[channel]+1u)%145u;
      }
    }
  }

void Master::set_prepared_pre_gain(float value) noexcept{pre_gain_=value;}

void Master::set_prepared_post_gain(float value) noexcept{
    stored_post_gain_=value;
    if(value!=target_) {
      target_=value;
      threshold_=std::max(std::fabs(value)*f(0x2b8cbcccu),f(0x1e3ce508u));
    }
  }

void Master::set_mono(bool value) noexcept{mono_=value;}

void Master::reset() noexcept{
    target_=stored_post_gain_;current_=stored_post_gain_;
    gain_buffer_.fill(stored_post_gain_);limiter_.reset();
  }

void Master::process(const StereoBlock& input,StereoBlock& output) noexcept{
    StereoBlock work{};
    for(unsigned ch=0;ch<2;++ch)for(unsigned i=0;i<16;++i)work[ch][i]=pre_gain_*input[ch][i];
    if(mono_) {
      for(auto& channel:work)for(auto& x:channel)x=mono_scale_*x;
      for(unsigned i=0;i<16;++i)work[0][i]=work[0][i]+work[1][i];
      work[1]=work[0];
    }
    limiter_.process(work,work);
    for(auto& channel:work)for(auto& x:channel) {
      if(x<= -1.0f)x=-1.0f;
      if(1.0f<=x)x=1.0f;
    }
    if(target_!=current_) {
      for(auto& gain:gain_buffer_) {
        const float delta=(target_-current_)*coefficient_;
        if(std::fabs(delta)<threshold_)current_=target_;
        else current_=current_+delta;
        gain=current_;
      }
    } else gain_buffer_.fill(target_);
    for(unsigned ch=0;ch<2;++ch)for(unsigned i=0;i<16;++i)output[ch][i]=gain_buffer_[i]*work[ch][i];
  }
}
