#include "mfrk/pitch_update.hpp"
#include <cmath>
namespace mfrk::pitch_update {
void advance(State& s) noexcept {
 for(unsigned voice=0;voice<6;++voice){
  if(!s.split&&!s.finished[voice]){
   const double step=s.increment[voice];double current=s.position[voice]+step;const double target=double(s.target[voice]);
   if((current<target&&step<0.)||(target<current&&step>0.)){current=target;s.finished[voice]=1;}
   s.position[voice]=current;s.dirty[voice]=1;s.dirty[voice+6]=1;
  }
  for(unsigned side=0;side<2;++side){
   const unsigned index=voice+6*side;
   if(s.split&&!s.finished[index]){
    const double step=s.increment[index];double current=s.position[index]+step;const double target=double(s.target[index]);
    if((current<target&&step<0.)||(target<current&&step>0.)){current=target;s.finished[index]=1;}
    s.position[index]=current;s.dirty[index]=1;
   }
   if(s.dirty[index]){
    const unsigned position_index=s.split?index:voice;
    const float bias=s.tune+s.bend;
    const float value=float(double(bias)+s.position[position_index]);
    s.output[index]=value;
    const float first=value+s.side_a[side];
    auto& slot=s.slots[voice*2+side];slot.dirty=1;slot.note=first+s.side_b[side];
    s.dirty[index]=0;
   }
  }
 }
}
bool setup_glide(State& s,GlideControl& c,unsigned index,bool copy_previous) noexcept {
 if(index>=12||(!s.split&&index>=6))return false;
 if(!c.enabled[index]||(c.gate_mode&&c.shared_gate)||c.force){
  s.position[index]=double(s.target[index]);s.finished[index]=1;s.dirty[index]=1;
  if(!s.split)s.dirty[index+6]=1;
  if(!c.held[index])c.shared_gate=0;
  return true;
 }
 const float rate=c.alternate?c.rate_b[index]:c.rate_a[index];
 if(!std::isfinite(rate)||rate<=0.f||(copy_previous&&c.last_index>=12))return false;
 if(copy_previous)s.position[index]=s.position[c.last_index];
 const double delta=double(s.target[index])-s.position[index];
 double increment=(0.0003333333333333333/double(rate))*delta;
 if(!c.scale_mode&&delta!=0.)increment=double(48.0f/std::fabs(float(delta)))*increment;
 s.increment[index]=increment;
 if(!c.dirty_a[index]&&!c.alternate&&!c.dirty_b[index]&&!c.gate_mode)s.finished[index]=1;
 else s.finished[index]=0;
 return true;
}
bool encode_note_pitch(std::uint8_t pitch,std::uint16_t& word) noexcept {
 if(pitch>127)return false;
 word=static_cast<std::uint16_t>((std::uint32_t(pitch)*65535u+64u)/128u);
 return true;
}
bool apply_note_word(State& state,GlideControl& control,unsigned index,std::uint16_t word) noexcept {
 if(index>=12||(!state.split&&index>=6))return false;
 State next=state;GlideControl next_control=control;
 next.target[index]=float((std::uint32_t(word)*129u+64u)>>16);
 if(!setup_glide(next,next_control,index,true))return false;
 next_control.force=0;next_control.last_index=index;
 state=next;control=next_control;return true;
}
bool apply_note_pitch(State& state,GlideControl& control,unsigned index,std::uint8_t pitch) noexcept {
 std::uint16_t word=0;if(!encode_note_pitch(pitch,word))return false;
 return apply_note_word(state,control,index,word);
}
}
