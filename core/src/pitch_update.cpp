#include "mfrk/pitch_update.hpp"
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
}
