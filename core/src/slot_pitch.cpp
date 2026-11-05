#include "mfrk/slot_pitch.hpp"
#include "ucrt_pitch_powf.hpp"
#include <bit>
#include <cmath>
namespace mfrk::slot_pitch {
namespace {
constexpr float f(std::uint32_t x)noexcept{return std::bit_cast<float>(x);}
 static constexpr std::array<unsigned,10> counts={12,1,2,7,7,7,5,5,5,5};
 static constexpr std::array<std::array<unsigned,12>,10> scales={{{0,1,2,3,4,5,6,7,8,9,10,11},{0},{0,7},{0,2,3,5,7,8,10},{0,2,4,5,7,9,11},{0,1,4,5,7,8,10},{0,2,3,7,10},{0,2,4,7,11},{0,3,5,7,10},{0,2,4,7,9}}};
}
Pitch::Pitch(ZeroTag)noexcept{put(0,std::uint64_t{0x1805533c8ull});}

bool Pitch::dirty(unsigned i)const noexcept{return get<std::uint8_t>(0x1a+i)!=0;}

float Pitch::norm(unsigned i)const noexcept{return static_cast<float>(get<std::uint16_t>(8+2*i))*f(0x37800200u);}

float Pitch::affine(unsigned i,std::uint32_t width,std::uint32_t minimum)const noexcept{return norm(i)*f(width)+f(minimum);}

void Pitch::prepare_records()noexcept{
  if(dirty(0)){const float x=affine(0,0x42f00000u,0xc2700000u);put(0x6d,std::uint8_t{1});put(0x50,x);put(0x34,x+get<float>(0x54));}
  if(dirty(1)){const float x=affine(1,0x42f00000u,0xc2700000u);put(0x6d,std::uint8_t{1});put(0x54,x);put(0x34,x+get<float>(0x50));}
  if(dirty(2)){const auto x=static_cast<std::uint16_t>((static_cast<std::uint32_t>(get<std::uint16_t>(12))*11u+5u)>>16);put(0x6d,std::uint8_t{1});put(0x38,x);put(0x6c,static_cast<std::uint8_t>(x!=0));}
  if(dirty(3)){
   const float n=norm(3),split=f(0x3daa829du);float transformed;
   if(n<.5f)transformed=(n/.5f)*split;
   else transformed=((n-.5f)/(1.0f-.5f))*(1.0f-split)+split;
   const float x=transformed*f(0x413ffbe7u)+f(0x3a83126fu);put(0x6d,std::uint8_t{1});put(0x3c,x);put(0x60,x*get<float>(0x40));
  }
  if(dirty(4)){const float x=static_cast<float>(static_cast<int>(get<std::uint16_t>(16))-128);put(0x6d,std::uint8_t{1});put(0x40,x);put(0x60,x*get<float>(0x3c));}
  if(dirty(5)){const float x=affine(5,0x40000000u,0xbf800000u);put(0x6d,std::uint8_t{1});put(0x44,x);put(0x64,(x*get<float>(0x48))*get<float>(0x4c));}
  if(dirty(6)){const float x=norm(6);put(0x6d,std::uint8_t{1});put(0x48,x);put(0x64,(x*get<float>(0x44))*get<float>(0x4c));}
  if(dirty(7)){const float x=affine(7,0x413e6666u,0x3dcccccdu);put(0x6d,std::uint8_t{1});put(0x4c,x);put(0x64,(get<float>(0x48)*get<float>(0x44))*x);}
  if(dirty(8)){put(0x6d,std::uint8_t{1});put(0x68,affine(8,0x3d75c28fu,0xbcf5c28fu));}
 }

Pitch Pitch::admitted_zero_owner_arena()noexcept{return Pitch(ZeroTag{});}

bool Pitch::try_prepare(const Input&in,Result&result)noexcept{
  if(!std::isfinite(in.base_note))return false;
  for(unsigned i=0;i<9;++i){put(8+2*i,in.words[i]);put(0x1a+i,in.dirty[i]);}
  put(0x30,in.base_note);put(0x6d,in.force);prepare_records();
  for(unsigned i=0;i<9;++i)put(0x1a+i,std::uint8_t{0});
  const bool updated=get<std::uint8_t>(0x6d)!=0;
  if(updated){
   float extra=get<float>(0x34);float note=(get<float>(0x64)+get<float>(0x60))+(get<float>(0x68)+get<float>(0x30));put(0x58,note);
   if(get<std::uint8_t>(0x6c)!=0){
    if(extra==0.0f)extra=0.0f;
    else{
     unsigned octaves=0;float direction;
     if(extra<=0.0f){do{extra=extra+12.0f;++octaves;}while(extra<=0.0f);direction=-1.0f;}
     else{while(extra>=12.0f){extra=extra-12.0f;++octaves;}direction=1.0f;}
     const unsigned row=get<std::uint16_t>(0x38)-1u;const auto count=counts[row];auto index=static_cast<std::uint64_t>((static_cast<float>(count)*extra)*f(0x3daaaaabu)+.5f);
     if(index>=count){octaves+=direction>0.0f?1u:static_cast<unsigned>(-1);index%=count;}
     extra=static_cast<float>(scales[row][static_cast<unsigned>(index)])+(static_cast<float>(octaves)*direction)*12.0f;
    }
   }
   note=note+extra;float clamped=0;if(note>=0.0f){clamped=note;if(note>=127.0f)clamped=127.0f;}
   put(0x58,clamped);const float exponent=(clamped-69.0f)*f(0x3daaaaabu);const float power=mfrk::detail::ucrt_pitch_powf::evaluate(exponent);put(0x6d,std::uint8_t{0});put(0x5c,power*440.0f);
  }
  result={updated,get<float>(0x58),get<float>(0x5c)};return true;
 }
}
