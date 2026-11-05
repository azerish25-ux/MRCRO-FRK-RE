#include "voice_audio_test_access.hpp"
#include "mfrk/bass.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace mfrk::voice_audio;

template<class T>T read(std::istream&in){T x{};if(!in.read(reinterpret_cast<char*>(&x),sizeof x))throw std::runtime_error("truncated");return x;}
struct ActualBassPair {
 mfrk::Bass first{mfrk::BassState::admitted({0,0,0},{0,0,0})};
 mfrk::Bass second{mfrk::BassState::admitted({0,0,0},{0,0,0})};
 std::array<bool,2> active{};
 mfrk::Bass& model(unsigned side)noexcept{return side==0?first:second;}
 void prepare()noexcept{first.prepare_controls(true);second.prepare_controls(true);}
};
struct Callback {
 ActualBassPair* models=nullptr;
 std::array<std::uint32_t,32>a,b,input;
 unsigned calls=0,errors=0;
 static void render(void*ctx,unsigned side,const void*in,std::span<float,32>output)noexcept{
  auto&c=*static_cast<Callback*>(ctx);c.errors+=side!=(c.calls%2);++c.calls;
  if(side==1){if(!in){++c.errors;return;}const auto* p=static_cast<const float*>(in);for(unsigned i=0;i<32;++i)c.errors+=std::bit_cast<std::uint32_t>(p[i])!=c.input[i];}
  else c.errors+=in!=nullptr;
  if(side==0)c.models->first.render_scaled(output);else c.models->second.render_scaled(output);
  const auto&source=side==0?c.a:c.b;for(unsigned i=0;i<32;++i)c.errors+=std::bit_cast<std::uint32_t>(output[i])!=source[i];
 }
};
using NativeModel=std::array<std::uint8_t,128>;
template<class T>T field(const NativeModel&b,unsigned offset){T x;std::memcpy(&x,b.data()+offset,sizeof x);return x;}
unsigned check_model(const ActualBassPair& pair,unsigned side,const NativeModel& b){
 const auto&s=(side==0?pair.first:pair.second).state();unsigned errors=0;
 for(unsigned j=0;j<3;++j){errors+=s.words[j]!=field<std::uint16_t>(b,8+2*j);errors+=s.dirty[j]!=b[14+j];errors+=std::bit_cast<std::uint32_t>(s.target[j])!=field<std::uint32_t>(b,0x58+4*j);errors+=std::bit_cast<std::uint32_t>(s.smoothed[j])!=field<std::uint32_t>(b,0x38+4*j);}
 const std::array<float,7> values={s.period,s.copied_frequency,s.frequency,s.phase_a,s.phase_b,s.phase_seed,s.level};
 const std::array<unsigned,7> offsets={0x30,0x34,0x54,0x44,0x48,0x64,0x28};
 for(unsigned j=0;j<7;++j)errors+=std::bit_cast<std::uint32_t>(values[j])!=field<std::uint32_t>(b,offsets[j]);
 errors+=s.random!=field<std::uint32_t>(b,0x4c);errors+=s.second_random!=field<std::uint32_t>(b,0x50);errors+=s.trigger_suppressed!=b[0x68];errors+=pair.active[side]!=(b[0x78]!=0);return errors;
}
int main(int argc,char**argv)try{
 if(argc!=2)return 2;
 std::ifstream in(argv[1],std::ios::binary);std::string magic(8,' ');in.read(magic.data(),8);if(magic!="MFVBAS02")throw std::runtime_error("magic");const auto count=read<std::uint32_t>(in);if(!count||count>10000)throw std::runtime_error("count");auto voice=VoiceAudio::admitted_zero_owner_arena();unsigned state_errors=0,audio_errors=0,fm_errors=0,callback_errors=0,model_errors=0;ActualBassPair models;const auto initial=read<std::array<std::uint32_t,154>>(in);if(initial!=TestAccess::snapshot(voice))throw std::runtime_error("constructor");
 for(unsigned side=0;side<2;++side)model_errors+=check_model(models,side,read<NativeModel>(in));
 const auto operations=read<std::uint32_t>(in);if(operations!=20)throw std::runtime_error("setupcount");
 for(unsigned op=0;op<operations;++op){
  const auto side=read<std::uint8_t>(in),kind=read<std::uint8_t>(in);const auto a=read<std::uint32_t>(in),b=read<std::uint32_t>(in),x=read<std::uint32_t>(in),y=read<std::uint32_t>(in);(void)y;if(side>1)throw std::runtime_error("setupside");auto& model=models.model(side);
  if(kind==1){if(a!=side||b!=(side==0?11u:12u))throw std::runtime_error("selection");models.active[side]=true;model.set_phase(0.f);model.initialize();}
  else if(kind==2){if(a>2||b>65535)throw std::runtime_error("word");auto w=model.state().words;w[a]=static_cast<std::uint16_t>(b);model.set_inputs(w,model.state().dirty);}
  else if(kind==3){if(a>2||b>255)throw std::runtime_error("dirty");auto dirty=model.state().dirty;dirty[a]=static_cast<std::uint8_t>(b);model.set_inputs(model.state().words,dirty);}
  else if(kind==4)model.set_pitch(std::bit_cast<float>(x));
  else if(kind==5){if(a>255)throw std::runtime_error("trigger");model.trigger(static_cast<std::uint8_t>(a));}
  else if(kind==6)model.prepare_controls(true);
  else throw std::runtime_error("setupopcode");
  for(unsigned target=0;target<2;++target)model_errors+=check_model(models,target,read<NativeModel>(in));
 }

 const auto check=[&](const auto&expected,unsigned n){const auto actual=TestAccess::snapshot(voice);for(unsigned i=0;i<154;++i)if(actual[i]!=expected[i]){if(state_errors<12)std::cerr<<"block "<<n<<" field "<<i<<" actual "<<std::hex<<actual[i]<<" expected "<<expected[i]<<std::dec<<'\n';++state_errors;}};
 for(unsigned n=0;n<count;++n){Controls c{};c.volume_word=read<std::uint16_t>(in);c.fm_word=read<std::uint16_t>(in);c.cutoff_word=read<std::uint16_t>(in);c.volume_dirty=read<std::uint8_t>(in);c.fm_dirty=read<std::uint8_t>(in);const auto para=read<std::uint8_t>(in),pre=read<std::uint8_t>(in);c.selected_lane=read<std::uint8_t>(in);if(para>1||pre>1||c.selected_lane>1)throw std::runtime_error("flags");c.para=para!=0;c.pre_level=pre!=0;c.mixer_words=read<std::array<std::uint16_t,4>>(in);c.mixer_dirty=read<std::array<std::uint8_t,4>>(in);const auto prepared=read<std::array<std::uint32_t,154>>(in);const auto prepared_models=read<std::array<NativeModel,2>>(in);Callback cb{};cb.models=&models;cb.a=read<std::array<std::uint32_t,32>>(in);cb.b=read<std::array<std::uint32_t,32>>(in);cb.input=read<std::array<std::uint32_t,32>>(in);const auto main=read<std::array<std::uint32_t,32>>(in),fm=read<std::array<std::uint32_t,32>>(in);const auto after_models=read<std::array<NativeModel,2>>(in);const auto after=read<std::array<std::uint32_t,154>>(in);if(!voice.prepare(c))throw std::runtime_error("prepare");check(prepared,n);models.prepare();for(unsigned side=0;side<2;++side)model_errors+=check_model(models,side,prepared_models[side]);std::array<float,32>m{},f{};if(!voice.render({&cb,Callback::render},nullptr,m,f))throw std::runtime_error("render");for(unsigned i=0;i<32;++i){audio_errors+=std::bit_cast<std::uint32_t>(m[i])!=main[i];fm_errors+=std::bit_cast<std::uint32_t>(f[i])!=fm[i];}callback_errors+=cb.errors+(cb.calls!=2);check(after,n);for(unsigned side=0;side<2;++side)model_errors+=check_model(models,side,after_models[side]);}
 if(in.peek()!=EOF)throw std::runtime_error("trailing");
 std::cout<<"blocks "<<count<<" state "<<state_errors<<" mixed "<<audio_errors<<" FM "<<fm_errors<<" callbacks "<<callback_errors<<" actualmodelstate "<<model_errors<<'\n';return state_errors||audio_errors||fm_errors||callback_errors||model_errors?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
