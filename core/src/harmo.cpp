#include "mfrk/harmo.hpp"
#include "ucrt_sinf.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>

namespace mfrk::harmo {
namespace {
inline float F(std::uint32_t x) { return std::bit_cast<float>(x); }
inline std::int16_t s16(std::int64_t x) { return std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(x)); }
inline std::int32_t s32(std::int64_t x) { return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(x)); }
inline std::int32_t product(std::int32_t a,std::int32_t b) { return s32(static_cast<std::int64_t>(a)*b); }
// Processor SHA256 bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74.
// Original static coefficient input VA0x18046c0d0, 8x9signed16.
inline constexpr std::int16_t coefficients[8][9]={{16384,3276,0,0,0,0,0,0,0},{16384,8192,3276,0,0,0,0,0,0},{16384,3276,8192,3276,0,0,0,0,0},{16384,3276,3276,8192,3276,0,0,0,0},{16384,3276,3276,3276,8192,3276,0,0,0},{16384,3276,3276,3276,3276,8192,3276,0,0},{16384,3276,3276,3276,3276,3276,8192,3276,0},{16384,8192,3276,3276,3276,3276,3276,8192,0}};
inline std::int32_t cvtt32(float x){return !std::isfinite(x)||x>=2147483648.0f||x< -2147483648.0f?INT32_MIN:static_cast<std::int32_t>(x);}
inline std::int64_t cvtt64(float x){const double value=x;return !std::isfinite(x)||value>=9223372036854775808.0||value< -9223372036854775808.0?INT64_MIN:static_cast<std::int64_t>(x);}
inline bool frequency_in_domain(float hz){return std::isfinite(hz)&&hz>0.0f;}
}
Tables::Tables() noexcept {
  static_assert(std::endian::native==std::endian::little);
  for(int row=0;row<8;++row) {
   float energy=0;
   for(int j=0;j<256;++j) {const float phase=static_cast<float>(j)*F(0x3cc90fdbu);float signal=0;
    for(int h=1;h<=8;++h) {const float arg=static_cast<float>(h)*phase;const float sine=mfrk::detail::ucrt_sinf::evaluate(arg);const float weighted=sine*static_cast<float>(coefficients[row][h-1]);signal=signal+weighted;}
    const float square=signal*signal;energy=energy+square;
   }
   const float mean=energy*F(0x3b800000u);const float rms=std::sqrt(mean);norm[row]=static_cast<std::int32_t>(805306368.0/static_cast<double>(rms));
  }
  std::array<double,2058> sine{},square{},saw{};std::uint32_t offset=0,shift=6;
  for(int length=1024,index=0;length>=2;length/=2,++index,++shift) {
   put(index*4,offset);put(40+index*4,shift);
   for(int j=0;j<length;++j) {
    const double phase=static_cast<double>(j)*(1.0/static_cast<double>(length));sine[offset+j]=std::sin(phase*6.283185307179586);
    double sum=0;for(int h=1;h<length/2;h+=2){const double arg=(static_cast<double>(h)*phase)*6.283185307179586;const double term=(std::sin(arg)*1.2732395447351628)/static_cast<double>(h);sum=sum+term;}square[offset+j]=sum;
    sum=0;for(int h=1;h<length/2;++h){const double arg=(static_cast<double>(h)*phase)*6.283185307179586;const double term=(std::sin(arg)*.6366197723675814)/static_cast<double>(h);sum=(h&1)?sum+term:sum-term;}saw[offset+j]=sum;
   }
   sine[offset+length]=sine[offset];square[offset+length]=square[offset];saw[offset+length]=saw[offset];offset+=length+1;
  }
  for(int j=0;j<2058;++j) {const auto sq=s16(static_cast<std::int32_t>(square[j]*4096.0));put(80+j*4,sq);put(80+j*4+2,s16(static_cast<std::int32_t>(sine[j]*8192.0)));put(80+8232+j*4,sq);put(80+8232+j*4+2,s16(static_cast<std::int32_t>(saw[j]*6144.0)));}
 }
Engine::Engine(const Tables&t,bool enabled,std::int16_t* external,bool initialize_now) noexcept:tables(t),external_delay(external) {
  put(0,std::uint64_t{0x180546928ull});put(0x20,F(0x43dc0000u));put(0x24,F(0x3b962fc9u));put(0x28,1.0f);
  for(int base:{0x158,0x178}){put(base,std::uint64_t{0x180546078ull});put(base+12,F(0x2b8cbcccu));put(base+28,F(0x2b8cbcccu));}
  put(0x16c,F(0x3c23d70au));put(0x170,F(0x3e1d33e4u));put(0x18c,F(0x3d4ccccdu));put(0x190,F(0x3d064870u));put(0x198,std::uint16_t{0x100});
  // Canonical pointer tokens correspond to the retained emulator's explicit arena.
  if(initialize_now) {bind_buffer(enabled,0);initialize();}
  // Aggregate owner ctor2e76a3/2e76b8 overrides raw primary vtable and gain.
  else {put(0,std::uint64_t{0x180544cc8ull});put(0x28,F(0x4023d70au));}
 }
void Engine::word(std::size_t o,std::int64_t x) noexcept {put(o,s16(x));}
void Engine::integer(std::size_t o,std::int64_t x) noexcept {put(o,s32(x));}
void Engine::bind_buffer(bool enabled,std::uint32_t count,std::uint64_t identity_label) noexcept {put(0x30,enabled?identity_label:std::uint64_t{0});put(0x38,count);}
void Engine::initialize() noexcept {put(0x120,get<std::uint64_t>(0x30));for(int i=0;i<5;++i)word(0x140+2*i,0x4000);}
void Engine::trigger(bool value) noexcept {if(value)put(0x89,std::uint8_t{1});}
void Engine::set_frequency(float value) noexcept {assert(frequency_in_domain(value));put(0x80,value);}
void Engine::set_phase(float phase) noexcept {assert(std::isfinite(phase));const float scaled=phase*4294967296.0f;put(0x138,static_cast<std::uint32_t>(cvtt64(scaled)));}
void Engine::reset() noexcept {
  for(int base:{0x158,0x178}) {const float x=get<float>(base==0x158?0x19c:0x1a0);put(base+8,x);put(base+12,x);}
  put(0x198,std::uint8_t{1});
}
bool Engine::set_word(unsigned index,std::uint16_t value) noexcept {
  if(index>=3)return false;
  put(8+2*index,value);return true;
}
bool Engine::set_dirty(unsigned index,std::uint8_t value) noexcept {
  if(index>=3)return false;
  put(14+index,value);return true;
}
void Engine::prepare_current() noexcept {
  Input in{};in.frequency=get<float>(0x80);in.gain=get<float>(0x28);
  for(unsigned i=0;i<3;++i) {in.words[i]=get<std::uint16_t>(8+2*i);in.dirty[i]=get<std::uint8_t>(14+i);}
  prepare(in);put(14,std::uint16_t{0});put(16,std::uint8_t{0});
}
void Engine::target(int base,float x) noexcept {if(x!=get<float>(base+8)){put(base+8,x);put(base+28,std::max(std::abs(x)*F(0x2b8cbcccu),F(0x1e3ce508u)));}}
float Engine::smoother(int base) noexcept {const float aim=get<float>(base+8),old=get<float>(base+12);const float delta=(aim-old)*get<float>(base+24);const float result=std::abs(delta)<get<float>(base+28)?aim:old+delta;put(base+12,result);return result;}
void Engine::prepare(const Input&in) noexcept {
  assert(frequency_in_domain(in.frequency)&&std::isfinite(in.gain));
  put(0x80,in.frequency);put(0x28,in.gain);if(in.trigger)put(0x89,std::uint8_t{1});
  if(in.reset_smoothers)reset();
  for(int j=0;j<3;++j){put(8+2*j,in.words[j]);put(14+j,in.dirty[j]);}
  if(in.dirty[0]){const float x=static_cast<float>(in.words[0])*F(931136000);put(0x19c,x);target(0x158,x);}
  if(in.dirty[1]){const int x=in.words[1]>>1;word(0x140,3276+((x*29491)>>15));word(0x144,16383+((x*-16383)>>15));}
  if(in.dirty[2]){const float x=static_cast<float>(in.words[2])*F(931136000);put(0x1a0,x);target(0x178,x);}
  if(get<float>(0x160)!=get<float>(0x164)||get<std::uint8_t>(0x198)){const int x=static_cast<int>(smoother(0x158)*32767.0f);word(0x146,x);word(0x148,19660+((s16(x)*-13107)>>15));}
  if(get<float>(0x180)!=get<float>(0x184)||get<std::uint8_t>(0x198))word(0x142,static_cast<int>(smoother(0x178)*32767.0f));
  put(0x198,std::uint8_t{0});
 }
void Engine::controls() noexcept {const float shape=static_cast<float>(get<std::int16_t>(0x140))*F(939524096),center=static_cast<float>(get<std::int16_t>(0x146))*F(939524096);const int q=static_cast<int>(shape*229376.0f),row=q>>15,frac=q&32767,other=frac^32767,centerq=static_cast<int>(center*229376.0f);
  auto interp=[](int r,int idx,int t){return s16((coefficients[r][idx]*(32767-t)+coefficients[r][idx+1]*t)>>15);};
  for(int h=0;h<8;++h){const int distance=std::abs((h<<15)-centerq),idx=distance>>15,rem=distance&32767;const auto next=idx>=8?std::int16_t{0}:s16((interp(row,idx,rem)*other+(frac?interp(row+1,idx,rem):0)*frac)>>15);const auto old=get<std::int16_t>(0x100+2*h);word(0xf0+2*h,old-next);word(0x100+2*h,next);}
  const auto next=frac?tables.norm[row]+((static_cast<std::int64_t>(tables.norm[row+1]-tables.norm[row])*frac)>>15):tables.norm[row];integer(0x110,get<>(0x114)-next);integer(0x114,next);
 }
int Engine::pitch() noexcept {const float hz=get<float>(0x80);put(0x94,static_cast<std::uint32_t>(cvtt64(hz*F(1202635582))));const int wave=get<std::int16_t>(0x144),bank=wave>=16384?1:0;integer(0x98,bank?2*wave-32768:32767-2*wave);put(0xa0,bank?std::uint64_t{0x1805b7838ull}:std::uint64_t{0x1805b5810ull});integer(0xe8,std::min(8,cvtt32(10800.0f/hz)));float x=hz*F(1018086206);const float inc=x+x;
  for(int h=0;h<8;++h){const auto z=std::bit_cast<std::uint32_t>(cvtt32(x));const int idx=static_cast<int>(std::bit_width(z));put(0xa8+4*h,tables.at<std::uint32_t>(40+4*idx));put(0xc8+4*h,tables.at<std::uint32_t>(4*idx));x=x+inc;}return bank;
 }
std::array<std::int16_t,16> Engine::harmonic(int bank) noexcept {auto phase=get<std::uint32_t>(0x90);const auto inc=get<std::uint32_t>(0x94);const int mix=get<>(0x98);std::array<std::int16_t,16> out{};
  for(int j=0;j<16;++j){const int n=15-j;auto hp=phase;const auto step=2*phase;std::int32_t total=0;
   for(int h=0;h<get<>(0xe8);++h){const auto z=hp>>(get<std::uint32_t>(0xa8+4*h)&31);hp+=step;const auto k=((z>>15)&0x1fffe)+2*get<std::uint32_t>(0xc8+4*h),t=(z>>1)&32767;auto sample=[&](int i){return tables.at<std::int16_t>(80+bank*8232+(k+i)*2);};
    const int a=s32(static_cast<std::int64_t>(sample(0))*mix+static_cast<std::int64_t>(sample(1))*(32767-mix))>>16,c=s32(static_cast<std::int64_t>(sample(2))*mix+static_cast<std::int64_t>(sample(3))*(32767-mix))>>16;const int wave=s32(static_cast<std::int64_t>(a)*(32767-t)+static_cast<std::int64_t>(c)*t)>>14;const int amp=(get<std::int16_t>(0xf0+2*h)*n>>3)+get<std::int16_t>(0x100+2*h);total=s32(static_cast<std::int64_t>(total)+(product(wave,amp)>>15));
   }
   phase+=inc;const int gain=(product(n,get<>(0x110))>>3)+get<>(0x114);const auto sample=(static_cast<std::int64_t>(gain)*total)>>15;out[j]=s16(std::clamp<std::int64_t>(sample,-16384,16383));
  }put(0x90,phase);return out;
 }
void Engine::delay_process(std::array<std::int16_t,16>&values) noexcept {
  auto memory=delay_words();if(!get<std::uint64_t>(0x120))return;
  for(int j=0;j<16;++j){const int input=values[j];const auto index=get<std::uint32_t>(0x118),slot=index&2047;const auto param=s16(get<std::int16_t>(0x12a)+(get<std::int16_t>(0x128)*(15-j))/16);memory[slot]=s16(get<std::int16_t>(0x12c)*input>>15);put(0x118,index+1);auto valid=get<std::uint32_t>(0x130);if(slot>=valid)++valid;put(0x130,valid);const auto lfo=get<std::uint32_t>(0x11c)>>16;
   auto quadratic=[](std::uint32_t p){p&=65535;const int sign=p&32768?-1:0;const int x=static_cast<int>(p&32767)-16384;return s16(((x*x>>13)^sign)-32768);};
   auto read=[&](int wave){const auto pos=s32(static_cast<std::int64_t>(static_cast<std::uint32_t>((index-1024)<<16))-(product(wave,param)>>5));auto k=(static_cast<std::uint32_t>(pos)>>16)&2047;const auto t=static_cast<std::uint32_t>(pos)&65535;const int a=k<valid?memory[k]:0;k=(k+1)&2047;const int c=k<valid?memory[k]:0;return s16(a+(product(c-a,static_cast<int>(t))>>16));};
   const int a=read(quadratic(lfo)),c=read(quadratic(lfo+16384));values[j]=s16(input+(product(a+c-input,param)>>16));put(0x11c,get<std::uint32_t>(0x11c)+0x346du);
  }
 }
std::array<std::int16_t,32> Engine::render_q15() noexcept {std::array<std::int16_t,32> out{};
  if(get<>(0x84)){integer(0x84,0);std::fill(state.begin()+0xf0,state.begin()+0x120,0);put(0x90,get<std::uint32_t>(0x138));integer(0xe8,8);std::fill(state.begin()+0x128,state.begin()+0x12e,0);integer(0x130,0);controls();put(0x88,std::uint8_t{1});word(0x8a,0);}
  else {const auto bank=pitch();const int old=get<std::int16_t>(0x12a),next=get<std::int16_t>(0x142);word(0x128,old-next);word(0x12a,next);word(0x12c,std::min(32767,get<std::int16_t>(0x12c)+48));controls();auto values=harmonic(bank);delay_process(values);int last=get<std::int16_t>(0x8a);const bool fadeout=get<std::uint8_t>(0x89),fadein=get<std::uint8_t>(0x88);
   for(int j=0;j<16;++j){const int x=values[j],mid=(last+x)>>1,factor=fadeout?16-j:(fadein?j:16);out[2*j]=s16(static_cast<int>(static_cast<float>(mid*factor)*.0625f));out[2*j+1]=s16(static_cast<int>(static_cast<float>(x*factor)*.0625f));last=x;}
   if(fadeout){put(0x89,std::uint8_t{0});integer(0x84,3);}else if(fadein)put(0x88,std::uint8_t{0});word(0x8a,last);
  }
  for(int j=0;j<32;++j){word(0x3c+2*j,out[j]);}
  std::copy_n(state.begin()+0x140,10,state.begin()+0x14a);return out;
 }
std::array<float,32> Engine::render_decoded() noexcept {
    const auto raw=render_q15();std::array<float,32> output{};
    for(unsigned i=0;i<32;++i)output[i]=static_cast<float>(raw[i])*F(0x38000100u);
    return output;
}
std::array<float,32> Engine::render_scaled() noexcept {
    auto output=render_decoded();const float gain=get<float>(0x28);
    for(auto& value:output)value=value*gain;
    return output;
}
std::array<float,32> Engine::process(const Input& input) noexcept {
    assert(frequency_in_domain(input.frequency)&&std::isfinite(input.gain));
    prepare(input);return render_scaled();
}
} // namespace mfrk::harmo
