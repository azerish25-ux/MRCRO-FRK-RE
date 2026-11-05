#include "mfrk/audio_block_adapter.hpp"
#include <algorithm>
namespace mfrk::audio {
bool BlockAdapter::fill(std::span<float> output) noexcept {
 if(output.size()%2!=0)return false;
 for(std::size_t frame=0;frame<output.size()/2;++frame){
  if(position_==16){
   if(!render_||!render_(context_,left_,right_)){std::fill(output.begin()+static_cast<std::ptrdiff_t>(2*frame),output.end(),0.f);return false;}
   position_=0;
  }
  output[2*frame]=left_[position_];output[2*frame+1]=right_[position_];++position_;
 }
 return true;
}
}
