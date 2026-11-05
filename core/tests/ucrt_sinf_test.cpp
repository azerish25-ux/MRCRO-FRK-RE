#include "ucrt_sinf.hpp"
#include <bit>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
std::uint32_t word(std::ifstream& in) {
  std::uint32_t result=0;
  for (unsigned i=0;i<4;++i) { int c=in.get();if(c<0)throw std::runtime_error("truncated fixture");result|=std::uint32_t(c)<<(8*i); }
  return result;
}
int main(int argc,char**argv) try {
  if(argc!=2)throw std::runtime_error("usage: sinf_test original_fixture.bin");
  std::ifstream in(argv[1],std::ios::binary);if(!in)throw std::runtime_error("fixture open");
  std::string magic(8,' ');in.read(magic.data(),8);if(magic!="UCSINF01" && magic!="UCSF0002")throw std::runtime_error("fixture magic");
  const auto count=word(in);if(count==0||count>1000000)throw std::runtime_error("invalid case count");std::uint64_t bad=0,host_differences=0;
  for(std::uint32_t i=0;i<count;++i) {
    const auto input=word(in),expected=word(in);const float x=std::bit_cast<float>(input);
    if(!mfrk::detail::ucrt_sinf::in_domain(x))throw std::runtime_error("fixture outside declared domain");
    const auto actual=std::bit_cast<std::uint32_t>(mfrk::detail::ucrt_sinf::evaluate(x));
    host_differences += std::bit_cast<std::uint32_t>(std::sin(x)) != expected;
    if(actual!=expected){if(bad<8)std::cerr<<"case "<<i<<" input "<<std::hex<<input<<" expected "<<expected<<" got "<<actual<<std::dec<<'\n';++bad;}
  }
  if(in.peek()!=EOF)throw std::runtime_error("trailing fixture");
  std::uint64_t admission_bad=0;
  for(auto b:{0x7f800000u,0xff800000u,0x7fc00000u,0xffc00000u,0x7f800001u})
    admission_bad+=mfrk::detail::ucrt_sinf::in_domain(std::bit_cast<float>(b));
  for(auto b:{0u,0x80000000u,0x4b800455u,0x4b800456u,0xcb800456u,0x7f7fffffu,0xff7fffffu})
    admission_bad+=!mfrk::detail::ucrt_sinf::in_domain(std::bit_cast<float>(b));
  std::cout<<"{\"cases\":"<<count<<",\"mismatches\":"<<bad<<",\"host_libm_differences\":"<<host_differences<<",\"admission_mismatches\":"<<admission_bad<<"}\n";
  return bad||admission_bad?1:0;
} catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
