#include "mfrk/waveshaper.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>

namespace mfrk::waveshaper {
namespace {
float F(std::uint32_t x) noexcept {return std::bit_cast<float>(x);}
static_assert(std::endian::native==std::endian::little);
static_assert(sizeof(float)==4&&std::numeric_limits<float>::is_iec559);
}

Tables::Tables() noexcept {
    constexpr double pi=3.141592653589793238462643383279502884;
    for(int i=0;i<256;++i){coarse[i]=static_cast<float>(std::pow(2.0,(i-128)/12.0));fine[i]=static_cast<float>(std::pow(2.0,i/256.0/12.0));}
    for(int bank=0;bank<5;++bank){std::array<double,129> positive{};
      for(int i=0;i<=128;++i){const double x=i/128.0;double y=0;
        if(bank==0)y=i==0?0.0:i==128?1.0:std::acos(std::tan(std::atan(8.0)*(1.0-2.0*x))/8.0)/pi;
        else if(bank==1)y=std::acos(1.0-2.0*x)/pi;
        else if(bank==2)y=x;
        else if(bank==3)y=(1.0-std::cos(pi*x*1.5))*(1.0-std::cos(pi*std::min(1.0,4.0-4.0*x)))/4.5;
        else y=std::sin(pi*x*1.5);
        positive[i]=y;
      }
      for(int i=0;i<257;++i){const double value=i<128?-positive[128-i]:positive[i-128];shape[bank][i]=static_cast<std::int16_t>(std::nearbyint(value*32767.0));}
    }
    std::array<double,516> temporary{};double maximum=0;
    for(int i=0;i<516;++i){const double x=(i==515?514:i)/256.0-1.0;const double sine=std::sin(8.0*pi*x);const double window=std::pow(std::exp(-x*x*4.0),2.0);const double y=sine*window+std::atan(3.0*x)*(1.0-window);temporary[i]=y;maximum=std::max(maximum,std::abs(y));}
    for(int i=0;i<516;++i)fold[i]=static_cast<float>(temporary[i]/maximum);
  }

float Tables::frequency(float note) const noexcept {
    const float shifted=note-9.0f;float bounded=-128.0f;
    if(shifted>=bounded)bounded=std::min(shifted,127.0f);
    const float pitch=bounded+128.0f;const int index=static_cast<int>(pitch);const int fraction=static_cast<int>((pitch-static_cast<float>(index))*256.0f);
    return (fine[fraction]*coarse[index])*F(966143945);
  }

std::int32_t Engine::cvtt32(float x) noexcept {return !std::isfinite(x)||x>=2147483648.0f||x< -2147483648.0f?INT32_MIN:static_cast<std::int32_t>(x);}

Engine::Engine(const Tables& tables,ZeroArenaTag) noexcept:tables_(tables) {construct();}

void Engine::construct() noexcept {
    put(0,std::uint64_t{0x180547200ull});put(0x18,std::uint64_t{0x4000051b0ull});put(0x38,std::uint64_t{0x1805415c0ull});put(0xc8,std::uint64_t{0x1805451f8ull});
    put(0x20,F(0x43dc0000u));put(0x24,F(0x3b962fc9u));put(0x28,F(0x3f79999au));put(0x78,0.0f);put(0x30,std::uint16_t{0});put(0x32,std::uint8_t{0});put(0xd0,std::uint8_t{0});
  }

float Engine::integrated(float u,bool negative) noexcept {
    const float square=u*u;const float half=u*.5f;float base=.1875f-half;const float term=square*.375f;float fourth=square*square;base=base+term;fourth=fourth*.0625f;return negative?fourth-base:base-fourth;
  }

void Engine::oscillator(float frequency,float pw) noexcept {
    frequency=std::min(.25f,std::max(F(897988541),frequency));const float lower=std::abs(frequency)*2.0f;pw=std::min(1.0f-lower,std::max(lower,pw));
    float fr=get(0x64),pr=get(0x68),phase=get(0x50),next=get(0x54);const float fd=(frequency-fr)*.0625f,pd=(pw-pr)*.0625f;auto high=get<std::uint8_t>(0x60);
    for(int i=0;i<16;++i){fr=fr+fd;pr=pr+pd;float sample=next;next=0.0f;phase=phase+fr;const float down=1.0f/(1.0f-pr),up=1.0f/pr,total=down+up;
      if(static_cast<unsigned>(pr>phase)!=high){const float t=(phase-pr)/fr,discontinuity=total*fr;high=static_cast<std::uint8_t>(pr>phase);sample=sample-integrated(1.0f-t)*discontinuity;next=integrated(t,true)*discontinuity;}
      float wave;
      if(phase>=1.0f){phase=phase-1.0f;high=1;const float t=phase/fr,discontinuity=total*fr;wave=phase*up;sample=sample+integrated(1.0f-t)*discontinuity;next=next+integrated(t)*discontinuity;}
      else wave=high?phase*up:1.0f-(phase-pr)*down;
      next=next+wave;slope_[i]=(sample+sample)-1.0f;
    }
    put(0x50,phase);put(0x54,next);put(0x64,fr);put(0x68,pr);put(0x60,high);
  }

RenderStatus Engine::core() noexcept {
    const float f0=tables_.frequency(get(0x84)),morph=get(0x8c),shape=get(0x90),amount=get(0x88);const float pw=morph*F(1055286886)+.5f;oscillator(f0,pw);
    const float slope=std::abs(morph-.5f)*5.0f+3.0f;float q=1.0f-(slope*f0-.03125f)*F(1074301065);q=std::clamp(q,0.0f,1.0f);const float cube=(q*q)*q,shape_offset=shape-.5f;
    float q2=((std::abs(shape_offset)*cube)*F(1101958051)+F(1087468638))*slope;q2=q2*f0;q2=1.0f-(q2-F(1035611788));q2=std::clamp(q2,0.0f,1.0f);
    const float target_shape=shape_offset*cube+.5f;const float target_gain=((amount*F(1055622431))*q2)*(q2*q2)+F(1022739087);
    float old_shape=get(0x6c),old_gain=get(0x70);const float ds=(target_shape-old_shape)*.0625f,dg=(target_gain-old_gain)*.0625f;
    for(int i=0;i<16;++i){old_shape=old_shape+ds;old_gain=old_gain+dg;const float position=slope_[i]*127.0f+128.0f;const int integral=cvtt32(position);const float fraction=position-static_cast<float>(integral);const unsigned index=static_cast<unsigned>(integral)&255;
      const float s=old_shape*F(1082130013);const int bank=cvtt32(s);const float mix=s-static_cast<float>(bank);if(bank<0||bank>=5)return RenderStatus::shape_domain;
      const auto lookup=[&](int row){assert(row>=0&&row<=5);if(row==5)row=4;const float y0=static_cast<float>(tables_.shape[row][index])*F(939524096),y1=static_cast<float>(tables_.shape[row][index+1])*F(939524096);return y0+(y1-y0)*fraction;};
      const float y0=lookup(bank),y1=lookup(bank+1),y=y0+(y1-y0)*mix;const float fold_position=(y*old_gain)*512.0f+256.0f;const int k=cvtt32(fold_position);const float t=fold_position-static_cast<float>(k);if(k<0||k>512)return RenderStatus::fold_domain;
      const float v0=tables_.fold[k],v1=tables_.fold[k+1],v2=tables_.fold[k+2],v3=tables_.fold[k+3];const float tangent=(v2-v0)*.5f,difference=v1-v2,opposite=(v3-v1)*.5f,middle=difference+tangent,a=opposite+(middle+difference);float value=t*a;value=value-(a+middle);value=value*t;value=value+tangent;value=value*t;folded_[i]=value+v1;
    }
    put(0x6c,old_shape);put(0x70,old_gain);return RenderStatus::ok;
  }

Engine Engine::admitted_zero_arena(const Tables& tables) noexcept {return Engine(tables,ZeroArenaTag{});}

void Engine::initialize() noexcept {
    initialized_=true;
    const auto pointer=get<std::uint64_t>(0xb8);const auto count=static_cast<std::int64_t>(get<std::int32_t>(0xc0));put(0x98,pointer);put(0xa0,pointer);put(0xa8,count);put(0xb0,count);put(0x94,F(0x3f4fffffu));
    put(0x50,std::uint64_t{0x3f000000ull});put(0x70,std::uint64_t{0});put(0x58,std::uint64_t{0x3f800000ull});put(0x68,std::uint64_t{0x3f000000ull});put(0x60,std::uint8_t{1});put(0x64,F(0x3a83126fu));
  }

void Engine::reset() noexcept {}

void Engine::set_phase(float) noexcept {}

void Engine::set_pitch(float /*unused_frequency*/,float pitch_argument) noexcept {put(0x84,pitch_argument);}

void Engine::trigger(bool gate) noexcept {put(0x30,static_cast<std::uint8_t>(gate));}

void Engine::bind_buffer(std::uint64_t opaque_token,std::int32_t count) noexcept {put(0xb8,opaque_token);put(0xc0,count);}

void Engine::refresh_controls(std::array<std::uint16_t,3> words) noexcept {for(unsigned j=0;j<3;++j)put(8+j*2,words[j]);}

void Engine::fetch_and_initialize(std::array<std::uint16_t,3> words) noexcept {set_controls(words,{1,1,1});initialize();}

std::uint8_t Engine::interface_flag() const noexcept {return get<std::uint8_t>(0xd0);}

void Engine::set_controls(std::array<std::uint16_t,3> words,std::array<std::uint8_t,3> dirty) noexcept {for(unsigned j=0;j<3;++j){put(8+j*2,words[j]);put(14+j,dirty[j]);}}

void Engine::prepare(bool interface_clear_dirty) noexcept {
    constexpr std::array<unsigned,3> offsets={0x90,0x88,0x8c};for(unsigned j=0;j<3;++j)if(get<std::uint8_t>(14+j))put(offsets[j],static_cast<float>(get<std::uint16_t>(8+2*j))*F(931136000));
    const auto gate=get<std::uint8_t>(0x30),latch=get<std::uint8_t>(0x31);std::int32_t pulse=0;
    if(!latch){if(gate){put(0x30,std::uint16_t{0x100});pulse=1;}}else if(!gate)put(0x31,std::uint8_t{0});put(0x80,pulse);
    if(interface_clear_dirty){put(14,std::uint16_t{0});put(16,std::uint8_t{0});}
  }

RenderStatus Engine::try_render_raw(std::array<float,32>& output) noexcept {
    if(!initialized_)return RenderStatus::not_initialized;
    const auto saved_state=state_;const auto saved_slope=slope_,saved_fold=folded_;
    const auto status=core();
    if(status!=RenderStatus::ok){state_=saved_state;slope_=saved_slope;folded_=saved_fold;return status;}
    std::array<float,32> result{};float previous=get(0x78);
    for(int i=0;i<16;++i){const float current=folded_[i];result[2*i]=(previous+current)*.5f;result[2*i+1]=current;previous=current;}
    put(0x78,previous);output=result;return RenderStatus::ok;
  }

RenderStatus Engine::try_render_scaled(std::array<float,32>& output) noexcept {
    std::array<float,32> result{};const auto status=try_render_raw(result);
    if(status!=RenderStatus::ok)return status;
    const float level=get(0x28);for(auto& value:result)value=value*level;output=result;return RenderStatus::ok;
  }

RenderStatus Engine::try_process(const Input& input,std::array<float,32>& output) noexcept {
    const auto saved_state=state_;const auto saved_slope=slope_,saved_fold=folded_;const bool saved_initialized=initialized_;
    set_controls(input.words,input.dirty);put(0x28,input.output_level);set_pitch(12345.0f,input.pitch_argument);trigger(input.gate);if(input.initialize)initialize();if(input.reset)reset();prepare(input.interface_prepare);
    const auto status=try_render_scaled(output);
    if(status!=RenderStatus::ok){state_=saved_state;slope_=saved_slope;folded_=saved_fold;initialized_=saved_initialized;}
    return status;
  }

bool Engine::try_set_control_word(unsigned index,std::uint16_t value) noexcept {
    if(index>=3)return false;
    put(8+index*2,value);return true;
  }

bool Engine::try_set_control_dirty(unsigned index,std::uint8_t value) noexcept {
    if(index>=3)return false;
    put(14+index,value);return true;
  }

} // namespace mfrk::waveshaper
