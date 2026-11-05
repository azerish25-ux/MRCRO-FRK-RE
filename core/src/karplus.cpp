#include "mfrk/karplus.hpp"
#include <algorithm>
#include <cmath>
#include "ucrt_sinf.hpp"
#include "ucrt_exp2f_compat.hpp"
namespace mfrk::karplus {
namespace {
inline float F(std::uint32_t x) noexcept{return std::bit_cast<float>(x);}
inline std::int16_t s16(std::int64_t x) noexcept{return std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(x));}
inline std::int32_t s32(std::int64_t x) noexcept{return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(x));}
inline std::int32_t cvtt(float x) noexcept{return !std::isfinite(x)||x>=2147483648.0f||x< -2147483648.0f?INT32_MIN:static_cast<std::int32_t>(x);}
inline std::int32_t clip14(std::int32_t x) noexcept{return std::clamp(x,-16384,16383);}
}
Tables::Tables() noexcept{
  constexpr std::array<double,2> periods={6.544984694978735e-05,3.2724923474893676e-05};
  for(int bank=0;bank<2;++bank){coefficients[bank*276]=1073741824;
   for(int note=1;note<137;++note){const float relative=static_cast<float>(note)-69.0f;const double exponent=static_cast<double>(relative)*0.08333333333333333;const double frequency=std::pow(2.0,exponent)*440.0;const double phase=frequency*periods[bank];const double gain=1.0/std::tan(phase);const double denom=gain+1.0;const double b=1.0/denom;const double a=(gain-1.0)*b;coefficients[bank*276+note]=static_cast<std::int32_t>(a*1073741824.0);coefficients[bank*276+138+note]=static_cast<std::int32_t>(b*1073741824.0);}
  }
 }

Engine::Engine(const Tables&t,SharedRandom&r,Memory&m,ZeroTag) noexcept:tables_(t),random_(r),memory_(m){construct();}

void Engine::integer(unsigned o,std::int64_t x)noexcept{put(o,s32(x));}

void Engine::short_word(unsigned o,std::int64_t x)noexcept{put(o,s16(x));}

void Engine::construct()noexcept{put(0,std::uint64_t{0x180547550ull});put(0x18,std::uint64_t{0x400030360ull});put(0x110,std::uint64_t{0x180545e40ull});put(0x20,F(0x43dc0000u));put(0x24,F(0x3b962fc9u));put(0x28,F(0x40086f69u));put(0x80,69.0f);put(0x88,std::uint64_t{0});std::fill(state_.begin()+0x90,state_.begin()+0x10c,0);put(0x118,std::uint8_t{0});}

Status Engine::coefficient_pair(float cutoff,int bank,std::int32_t&a,std::int32_t&b)noexcept{
  cutoff=std::min(cutoff,136.0f);const auto index=cvtt(cutoff);if(index<0||index>136)return Status::table_domain;const float fraction=cutoff-static_cast<float>(index);
  const auto interpolate=[&](int start){const float x=static_cast<float>(tables_.coefficients[start+index]),y=static_cast<float>(tables_.coefficients[start+index+1]);return cvtt(x+(y-x)*fraction);};a=interpolate(bank*276);b=interpolate(bank*276+138);return Status::ok;
 }

Status Engine::excite()noexcept{
  const float hz=get<float>(0x80),note=get<float>(0x88),p=static_cast<float>(get<std::int16_t>(0xf6))*F(939524096);std::int32_t a,b;auto status=coefficient_pair(note+p*96.0f,0,a,b);if(status!=Status::ok)return status;integer(0xb8,a);integer(0xbc,b);
  const auto delay=s32(-std::int64_t{cvtt(786432000.0f/hz+.5f)});const int fraction=delay&16383,phase=get<>(0x90),base=s32(std::int64_t{phase}+(delay>>14));const auto old=cvtt((static_cast<float>(get<std::int16_t>(0x10a))*F(939524096))*2147483648.0f),negative=cvtt((static_cast<float>(get<std::int16_t>(0xfc))*F(939524096))*-2147483648.0f),delta=s32(-std::int64_t{old}-negative);std::array<std::int32_t,16> temp{};auto previous=get<>(0xb0),filtered=get<>(0xb4);const bool off=p==0||get<std::uint8_t>(0xc0)!=0;
  for(int n=0;n<16;++n){random_.value=random_.value*0x41c64e6du+0x3039u;const auto noise=s16(s16(std::bit_cast<std::int32_t>(random_.value)>>15)>>5);auto&ring=memory_.samples;const auto pos=(static_cast<std::uint32_t>(base)+static_cast<unsigned>(n))&4095;const auto interp=s32(std::int64_t{ring[pos]}*(16384-fraction)+std::int64_t{ring[(pos+1)&4095]}*fraction)>>14;ring[(static_cast<std::uint32_t>(phase)+static_cast<unsigned>(n))&4095]=s16((interp*31675>>15)+noise);const auto total=s32(std::int64_t{previous}+interp);previous=interp;filtered=s32((std::int64_t{total}*b>>30)+(std::int64_t{filtered}*a>>30));const auto mix=s32(std::int64_t{old}+(std::int64_t{delta}*n>>4));temp[n]=off?0:s32(((std::int64_t{mix}^0x7fffffffll)*filtered)>>30);}
  integer(0xb0,previous);integer(0xb4,filtered);const int remaining=std::clamp(s32(std::int64_t{get<>(0x94)}-phase),0,16);float position=static_cast<float>(static_cast<std::uint32_t>(phase));const float inv=get<float>(0x98),fm=get<float>(0x9c),amp=get<float>(0xa0);
  for(int n=0;n<remaining;++n){const float theta=(position*inv)*F(1086918619),argument=theta*fm-F(1078530011),half=theta*.5f;if(!mfrk::detail::ucrt_sinf::in_domain(argument)||!mfrk::detail::ucrt_sinf::in_domain(half))return Status::math_domain;const float sinc=argument==0?1.0f:mfrk::detail::ucrt_sinf::evaluate(argument)/argument;const float envelope=std::min(mfrk::detail::ucrt_sinf::evaluate(half)*2.0f,1.0f);const auto value=std::clamp(cvtt((envelope*sinc)*amp),0,32767),mix=s32(std::int64_t{old}+(std::int64_t{delta}*n>>4));temp[n]=s32(std::int64_t{temp[n]}+(std::int64_t{mix}*value>>30));position=position+1.0f;}
  integer(0x90,std::int64_t{phase}+16);int last=get<std::int16_t>(0xc2);for(int n=0;n<16;++n){const int x=clip14(temp[n]);excitation_[2*n]=s16((last+x)>>1);excitation_[2*n+1]=s16(x);last=x;}short_word(0xc2,last);return Status::ok;
 }

Status Engine::string()noexcept{
  const float cutoff=get<float>(0x88)+(static_cast<float>(get<std::int16_t>(0xf4))*F(939524096))*120.0f;std::int32_t a,b;auto status=coefficient_pair(cutoff,1,a,b);if(status!=Status::ok)return status;integer(0xe4,a);integer(0xe8,b);const float gain=static_cast<float>(get<std::int16_t>(0xf2))*F(939524096),q=1.0f-gain;const auto feedback=cvtt((1.0f-q*q)*268435456.0f+805306368.0f),old_delay=cvtt(1572864000.0f/get<float>(0x84)+.5f),new_delay=cvtt(1572864000.0f/get<float>(0x80)+.5f),delta=s32(std::int64_t{old_delay}-new_delay);auto index=get<std::uint32_t>(0xc8);auto lp=get<>(0xd8),hp=get<>(0xdc),filtered=get<>(0xe0);
  for(int n=0;n<32;++n){const auto value=s32(((std::int64_t{filtered}*feedback+0x20000000)>>30)+(excitation_[n]>>1));memory_.samples[4096+(index&4095)]=s16(clip14(value));++index;const auto delay=s32((std::int64_t{delta}*n>>5)-old_delay),read=s32(std::int64_t{index}+(delay>>14));const int fraction=delay&16383;const auto pos=static_cast<std::uint32_t>(read)&4095;const auto sample=s32(std::int64_t{memory_.samples[4096+pos]}*(16384-fraction)+std::int64_t{memory_.samples[4096+((pos+1)&4095)]}*fraction)>>14;const auto previous=lp;lp=s32(std::int64_t{lp}+8+std::int64_t{sample}*15)>>4;filtered=s32((std::int64_t{s32(std::int64_t{previous}+lp)}*b>>30)+(std::int64_t{filtered}*a>>30));hp=s32((std::int64_t{hp}*0x3fae147a>>30)-previous+lp);raw_[n]=s16(clip14(hp));}
  put(0xc8,index);integer(0xd8,lp);integer(0xdc,hp);integer(0xe0,filtered);return Status::ok;
 }

Status Engine::core()noexcept{
  const auto counter=get<>(0x8c);excitation_.fill(0);
  if(counter>0){const auto next=s32(std::int64_t{counter}-1);integer(0x8c,next);if(next==16){integer(0xc8,0);put(0xdc,std::uint64_t{0});integer(0xd8,0);const float period=48000.0f/get<float>(0x80);const auto whole=cvtt(period);float rounded=period;if(whole!=INT32_MIN&&static_cast<float>(whole)!=period)rounded=static_cast<float>(s32(std::int64_t{whole}+(std::signbit(period)?0:1)));integer(0x90,0);put(0xb0,std::uint64_t{0});integer(0x94,cvtt(rounded));const float p=static_cast<float>(get<std::int16_t>(0xf8))*F(960495616),q=static_cast<float>(get<std::int16_t>(0xfa))*F(964689920);if(!std::isfinite(p)||p< -64||p>64||!std::isfinite(q)||q< -64||q>64)return Status::math_domain;put(0x9c,mfrk::detail::ucrt_exp2f::evaluate(p));put(0xa0,mfrk::detail::ucrt_exp2f::evaluate(q)*32768.0f);short_word(0xc2,0);put(0x98,1.0f/period);put(0x84,get<float>(0x80));}
   else{if(next<0||next>=16)return Status::math_domain;for(int bank=0;bank<2;++bank)std::fill_n(memory_.samples.begin()+bank*4096+next*256,256,std::int16_t{0});}raw_.fill(0);
  }else{auto status=excite();if(status!=Status::ok)return status;status=string();if(status!=Status::ok)return status;put(0x84,get<float>(0x80));}
  for(unsigned i=0;i<32;++i){short_word(0x3c+i*2,raw_[i]);}
  std::copy_n(state_.begin()+0xf0,14,state_.begin()+0xfe);return Status::ok;
 }

Engine Engine::admitted_zero_arena(const Tables&t,SharedRandom&r,Memory&m)noexcept{return Engine(t,r,m,ZeroTag{});}

void Engine::bind(std::uint64_t token,std::int32_t count)noexcept{put(0x30,token);put(0x38,count);}

void Engine::initialize()noexcept{const auto pointer=get<std::uint64_t>(0x30);put(0xa8,pointer);put(0xd0,pointer+8192);std::fill(state_.begin()+0xf0,state_.begin()+0xfe,0);initialized_=true;}

void Engine::reset()noexcept{}

void Engine::set_phase(float)noexcept{}

bool Engine::set_word(unsigned index,std::uint16_t value)noexcept{if(index>=3)return false;put(8+index*2,value);return true;}

bool Engine::set_dirty(unsigned index,std::uint8_t value)noexcept{if(index>=3)return false;put(14+index,value);return true;}

void Engine::set_flag(std::uint8_t value)noexcept{put(0x118,value);}

std::uint8_t Engine::flag()const noexcept{return get<std::uint8_t>(0x118);}

void Engine::fetch_words(const std::array<std::uint16_t,3>& words)noexcept{for(unsigned i=0;i<3;++i)put(8+i*2,words[i]);}

void Engine::initialize_from_records(const std::array<std::uint16_t,3>& words)noexcept{for(unsigned i=0;i<3;++i)put(14+i,std::uint8_t{1});fetch_words(words);initialize();}

void Engine::prepare_current(bool interface)noexcept{Input in{};for(unsigned i=0;i<3;++i){in.words[i]=get<std::uint16_t>(8+i*2);in.dirty[i]=get<std::uint8_t>(14+i);}in.interface_prepare=interface;prepare(in);}

void Engine::set_level(float value)noexcept{put(0x28,value);}

void Engine::set_pitch(float hz,float note)noexcept{if(!(note>=18.25f)){hz=F(1102643017);note=18.25f;}put(0x88,std::max(note,18.25f));put(0x80,hz);}

void Engine::trigger(bool on)noexcept{if(on){integer(0x8c,17);put(0xc0,std::uint8_t{0});}else put(0xc0,std::uint8_t{1});}

void Engine::prepare(const Input&in)noexcept{for(unsigned i=0;i<3;++i){put(8+i*2,in.words[i]);put(14+i,in.dirty[i]);}if(in.dirty[0]){const int x=in.words[0]>>1;short_word(0xf6,6553+(x*26214>>15));short_word(0xfc,32767+(x*-22937>>15));}if(in.dirty[1]){const int x=in.words[1]>>1,y=x*11469>>15;short_word(0xfa,13106+y);short_word(0xf8,8191+y);}if(in.dirty[2]){const int x=in.words[2]>>1;short_word(0xf2,13106+(x*19661>>15));short_word(0xf4,8191+((3*x<<13)>>15));}if(in.interface_prepare){put(14,std::uint16_t{0});put(16,std::uint8_t{0});}}

Status Engine::try_render(std::array<float,32>&output,bool scaled)noexcept{if(!initialized_)return Status::not_initialized;if(get<std::uint64_t>(0xa8)==0)return Status::unbound;const auto state=state_;const auto buffers=memory_.samples;const auto random=random_.value;const auto raw=raw_,excite=excitation_;auto status=core();if(status!=Status::ok){state_=state;memory_.samples=buffers;random_.value=random;raw_=raw;excitation_=excite;return status;}std::array<float,32> result{};for(unsigned i=0;i<32;++i){float x=static_cast<float>(raw_[i])*F(939524352);if(scaled)x=x*get<float>(0x28);result[i]=x;}output=result;return Status::ok;}

Status Engine::try_process(const Input&in,std::array<float,32>&output)noexcept{if(in.trigger_event<0||in.trigger_event>2)return Status::invalid_input;const auto state=state_;const auto buffers=memory_.samples;const auto random=random_.value;const auto raw=raw_,excite=excitation_;set_pitch(in.frequency_hz,in.note);put(0x28,in.level);if(in.trigger_event==2)trigger(true);else if(in.trigger_event==1)trigger(false);prepare(in);auto status=try_render(output);if(status!=Status::ok){state_=state;memory_.samples=buffers;random_.value=random;raw_=raw;excitation_=excite;}return status;}
}
