#include "ucrt_pitch_powf.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
std::uint32_t word(std::istream&in){std::uint32_t x=0;for(unsigned i=0;i<4;++i){int c=in.get();if(c<0)throw std::runtime_error("truncated");x|=std::uint32_t(c)<<(8*i);}return x;}
int main(int argc,char**argv)try{
 if(argc!=2)throw std::runtime_error("arguments");
 std::ifstream in(argv[1],std::ios::binary);if(!in)throw std::runtime_error("open");
 std::string magic(8,' ');in.read(magic.data(),8);if(magic!="MFPOW201")throw std::runtime_error("magic");
 const auto count=word(in);if(!count||count>1000000)throw std::runtime_error("count");unsigned mismatches=0,hz_mismatches=0,admission=0;
 for(unsigned i=0;i<count;++i){const auto input=word(in),expected=word(in),hz=word(in);float actual=0;const auto y=std::bit_cast<float>(input);if(!mfrk::detail::ucrt_pitch_powf::try_evaluate(y,actual))throw std::runtime_error("gold domain");mismatches+=std::bit_cast<std::uint32_t>(actual)!=expected;hz_mismatches+=std::bit_cast<std::uint32_t>(actual*440.0f)!=hz;}
 if(in.peek()!=EOF)throw std::runtime_error("trailing");
 for(auto b:{0x7f800000u,0xff800000u,0x7fc00000u,0xc0b80001u,0x409aaaacu}){float output=std::bit_cast<float>(0x7fa12345u);const auto before=std::bit_cast<std::uint32_t>(output);admission+=mfrk::detail::ucrt_pitch_powf::try_evaluate(std::bit_cast<float>(b),output);admission+=std::bit_cast<std::uint32_t>(output)!=before;}
 std::cout<<"cases "<<count<<" powf "<<mismatches<<" hz "<<hz_mismatches<<" admission "<<admission<<'\n';return mismatches||hz_mismatches||admission?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
