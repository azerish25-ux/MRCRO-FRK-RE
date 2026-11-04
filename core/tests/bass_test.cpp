#include "mfrk/bass.hpp"
#include <array>
#include <bit>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
static_assert(std::endian::native==std::endian::little);
using Bytes=std::array<std::uint8_t,128>;
template<class T>T get(const Bytes& b,unsigned offset){T x;std::memcpy(&x,b.data()+offset,sizeof x);return x;}
template<class T>void put(Bytes& b,unsigned offset,T x){std::memcpy(b.data()+offset,&x,sizeof x);}
mfrk::BassState decode(const Bytes& b){
 auto s=mfrk::BassState::admitted({get<std::uint16_t>(b,8),get<std::uint16_t>(b,10),get<std::uint16_t>(b,12)},{b[14],b[15],b[16]});
 s.period=get<float>(b,0x30);s.copied_frequency=get<float>(b,0x34);s.frequency=get<float>(b,0x54);s.phase_a=get<float>(b,0x44);s.phase_b=get<float>(b,0x48);s.phase_seed=get<float>(b,0x64);s.random=get<std::uint32_t>(b,0x4c);s.second_random=get<std::uint32_t>(b,0x50);s.trigger_suppressed=b[0x68];s.level=get<float>(b,0x28);
 for(unsigned j=0;j<3;++j){s.target[j]=get<float>(b,0x58+j*4);s.smoothed[j]=get<float>(b,0x38+j*4);}return s;
}
Bytes encode(const mfrk::BassState& s,Bytes b){
 for(unsigned j=0;j<3;++j){put(b,8+j*2,s.words[j]);b[14+j]=s.dirty[j];put(b,0x58+j*4,s.target[j]);put(b,0x38+j*4,s.smoothed[j]);}
 put(b,0x30,s.period);put(b,0x34,s.copied_frequency);put(b,0x54,s.frequency);put(b,0x44,s.phase_a);put(b,0x48,s.phase_b);put(b,0x64,s.phase_seed);put(b,0x4c,s.random);put(b,0x50,s.second_random);b[0x68]=s.trigger_suppressed;put(b,0x28,s.level);return b;
}
template<class T>void read(std::ifstream& f,T& x){f.read(reinterpret_cast<char*>(&x),sizeof x);if(!f){std::puts("bad fixture read");std::exit(2);}}
int main(int argc,char**argv){
 if(argc!=2){return 2;}
 std::ifstream file(argv[1],std::ios::binary);std::array<char,8> magic;read(file,magic);if(magic!=std::array<char,8>{'B','A','S','S','F','X','0','1'})return 2;std::uint32_t count;read(file,count);if(count!=776)return 2;std::array<unsigned,10> cases{},bad{};unsigned failures=0,samples=0;
 for(unsigned i=0;i<count;++i){std::uint32_t kind,flag,audio_size;float arg;read(file,kind);read(file,arg);read(file,flag);read(file,audio_size);if(kind>9 || (audio_size!=0 && audio_size!=128) || ((kind<2)!=(audio_size==128)))return 2;Bytes before,expected;read(file,before);read(file,expected);std::array<float,32> golden{},output{};if(audio_size)read(file,golden);auto s=decode(before);if(kind==9)s=mfrk::BassState::admitted(s.words,s.dirty);mfrk::Bass bass(s);
  switch(kind){case 0:bass.render_raw(output);break;case 1:bass.render_scaled(output);break;case 2:bass.prepare_controls();break;case 3:bass.prepare_controls(true);break;case 4:case 5:bass.initialize();break;case 6:bass.set_pitch(arg);break;case 7:bass.set_phase(arg);break;case 8:bass.trigger(static_cast<std::uint8_t>(flag));break;case 9:break;default:return 2;}
  const auto after=encode(bass.state(),before);const bool mismatch=after!=expected||(audio_size&&std::memcmp(output.data(),golden.data(),128));++cases[kind];samples+=audio_size?32:0;
  if(mismatch){++bad[kind];++failures;if(failures<4){std::printf("case%u kind%u mismatch",i,kind);for(unsigned j=0;j<128;++j)if(after[j]!=expected[j]){std::printf(" state+%x expected%02x got%02x",j,expected[j],after[j]);break;}for(unsigned j=0;j<32&&audio_size;++j)if(std::bit_cast<std::uint32_t>(output[j])!=std::bit_cast<std::uint32_t>(golden[j])){std::printf(" sample%u expected%08x got%08x",j,std::bit_cast<std::uint32_t>(golden[j]),std::bit_cast<std::uint32_t>(output[j]));break;}std::puts("");}}
 }
 if(file.peek()!=std::char_traits<char>::eof())return 2;
 for(unsigned k=0;k<10;++k){std::printf("kind%u cases%u mismatches%u\n",k,cases[k],bad[k]);}
 std::printf("TOTAL %u fixtures %u samples %u mismatches\n",count,samples,failures);return failures?1:0;
}
