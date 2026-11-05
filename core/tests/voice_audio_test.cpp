#include "voice_audio_test_access.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace mfrk::voice_audio;

template<class T>T read(std::istream&in){T x{};if(!in.read(reinterpret_cast<char*>(&x),sizeof x))throw std::runtime_error("truncated");return x;}
struct Callback {
 std::array<std::uint32_t,32>a,b,input;
 unsigned calls=0,errors=0;
 static void render(void*ctx,unsigned side,const void*in,std::span<float,32>output)noexcept{
  auto&c=*static_cast<Callback*>(ctx);c.errors+=side!=(c.calls%2);++c.calls;
  if(side==1){if(!in){++c.errors;return;}const auto* p=static_cast<const float*>(in);for(unsigned i=0;i<32;++i)c.errors+=std::bit_cast<std::uint32_t>(p[i])!=c.input[i];}
  else c.errors+=in!=nullptr;
  const auto&source=side==0?c.a:c.b;for(unsigned i=0;i<32;++i)output[i]=std::bit_cast<float>(source[i]);
 }
};
int main(int argc,char**argv)try{
 if(argc!=2)return 2;
 std::ifstream in(argv[1],std::ios::binary);std::string magic(8,' ');in.read(magic.data(),8);if(magic!="MFVAUD01")throw std::runtime_error("magic");const auto count=read<std::uint32_t>(in);if(!count||count>10000)throw std::runtime_error("count");auto voice=VoiceAudio::admitted_zero_owner_arena();const auto initial=read<std::array<std::uint32_t,154>>(in);if(initial!=TestAccess::snapshot(voice))throw std::runtime_error("constructor");unsigned state_errors=0,audio_errors=0,fm_errors=0,callback_errors=0;
 const auto check=[&](const auto&expected,unsigned n){const auto actual=TestAccess::snapshot(voice);for(unsigned i=0;i<154;++i)if(actual[i]!=expected[i]){if(state_errors<12)std::cerr<<"block "<<n<<" field "<<i<<" actual "<<std::hex<<actual[i]<<" expected "<<expected[i]<<std::dec<<'\n';++state_errors;}};
 for(unsigned n=0;n<count;++n){Controls c{};c.volume_word=read<std::uint16_t>(in);c.fm_word=read<std::uint16_t>(in);c.cutoff_word=read<std::uint16_t>(in);c.volume_dirty=read<std::uint8_t>(in);c.fm_dirty=read<std::uint8_t>(in);const auto para=read<std::uint8_t>(in),pre=read<std::uint8_t>(in);c.selected_lane=read<std::uint8_t>(in);if(para>1||pre>1||c.selected_lane>1)throw std::runtime_error("flags");c.para=para!=0;c.pre_level=pre!=0;c.mixer_words=read<std::array<std::uint16_t,4>>(in);c.mixer_dirty=read<std::array<std::uint8_t,4>>(in);const auto prepared=read<std::array<std::uint32_t,154>>(in);Callback cb{};cb.a=read<std::array<std::uint32_t,32>>(in);cb.b=read<std::array<std::uint32_t,32>>(in);cb.input=read<std::array<std::uint32_t,32>>(in);const auto main=read<std::array<std::uint32_t,32>>(in),fm=read<std::array<std::uint32_t,32>>(in);const auto after=read<std::array<std::uint32_t,154>>(in);if(!voice.prepare(c))throw std::runtime_error("prepare");check(prepared,n);std::array<float,32>m{},f{};if(!voice.render({&cb,Callback::render},nullptr,m,f))throw std::runtime_error("render");for(unsigned i=0;i<32;++i){audio_errors+=std::bit_cast<std::uint32_t>(m[i])!=main[i];fm_errors+=std::bit_cast<std::uint32_t>(f[i])!=fm[i];}callback_errors+=cb.errors+(cb.calls!=2);check(after,n);}
 if(in.peek()!=EOF)throw std::runtime_error("trailing");
 std::cout<<"blocks "<<count<<" state "<<state_errors<<" mixed "<<audio_errors<<" FM "<<fm_errors<<" callbacks "<<callback_errors<<'\n';return state_errors||audio_errors||fm_errors||callback_errors?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
