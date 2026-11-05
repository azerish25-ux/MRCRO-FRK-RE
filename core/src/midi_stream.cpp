#include "mfrk/midi_stream.hpp"

namespace mfrk::midi {
void Decoder::reset() noexcept {running_=status_=needed_=count_=0;data_={};sysex_=false;}
void Decoder::feed(std::span<const std::uint8_t> bytes,Receiver receiver,void* context) noexcept {
 if(!receiver)return;
 for(const auto byte:bytes){
  if(byte>=0xf8){receiver(context,Event{byte,{},0,false});continue;}
  if(byte&0x80){
   if(sysex_&&byte==0xf7){receiver(context,Event{byte,{},0,true});sysex_=false;continue;}
   sysex_=false;status_=needed_=count_=0;data_={};
   if(byte<0xf0){running_=byte;status_=byte;needed_=((byte&0xe0)==0xc0)?1:2;continue;}
   running_=0;
   if(byte==0xf0){sysex_=true;receiver(context,Event{byte,{},0,true});}
   else if(byte==0xf1||byte==0xf3){status_=byte;needed_=1;}
   else if(byte==0xf2){status_=byte;needed_=2;}
   else if(byte==0xf6||byte==0xf7)receiver(context,Event{byte,{},0,false});
   continue;
  }
  if(sysex_){receiver(context,Event{byte,{},0,true});continue;}
  if(!status_){if(!running_)continue;status_=running_;needed_=((running_&0xe0)==0xc0)?1:2;count_=0;data_={};}
  data_[count_++]=byte;
  if(count_==needed_){receiver(context,Event{status_,data_,needed_,false});status_=needed_=count_=0;data_={};}
 }
}
}
