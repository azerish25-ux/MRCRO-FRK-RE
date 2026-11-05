#include "master_test_access.hpp"
#include "karplus_fixture_reader.hpp"
#include <iostream>
#include <cstring>
#include <stdexcept>
using namespace mfrk::master;
struct Snapshot {
 std::array<std::uint32_t,4> controls,coefficients,limiter;
 std::array<std::uint32_t,6> cursors;
 std::array<std::uint32_t,4> smoother;
 std::array<std::uint32_t,16> gains;
 std::array<std::uint32_t,290> rings;
};
Snapshot snapshot(FixtureReader&r) {
 return {r.read<std::array<std::uint32_t,4>>(),r.read<std::array<std::uint32_t,4>>(),r.read<std::array<std::uint32_t,4>>(),r.read<std::array<std::uint32_t,6>>(),r.read<std::array<std::uint32_t,4>>(),r.read<std::array<std::uint32_t,16>>(),r.read<std::array<std::uint32_t,290>>()};
}
int main(int argc,char**argv)try {
 if(argc!=2)return 2;
 FixtureReader r(argv[1]);
 if(r.read<std::array<char,8>>()!=std::array<char,8>{'M','O','C','O','L','D','0','1'})return 2;
 const auto cases=r.read<std::uint32_t>();if(cases!=4)return 2;
 std::uint64_t states=0,rings=0,outputs=0,inputs=0,blocks=0;
 for(unsigned c=0;c<cases;++c) {
  const auto ready=r.read<std::uint8_t>(),inplace=r.read<std::uint8_t>();if(ready>1||inplace>1)return 2;
  const auto initial=snapshot(r);DelayMemory memory;
  for(unsigned ch=0;ch<2;++ch)for(unsigned i=0;i<145;++i)memory.samples[ch][i]=std::bit_cast<float>(initial.rings[ch*145+i]);
  auto master=ready?Master::admitted_ready_owner(memory):Master::admitted_zero_owner_arena(memory);
  const auto check=[&](const Snapshot&s) {
   const auto& l=MasterTestAccess::limiter(master);
   states+=MasterTestAccess::controls(master)!=s.controls;
   states+=LimiterTestAccess::coefficients(l)!=s.coefficients;
   states+=MasterTestAccess::smoother(master)!=s.smoother;
   for(unsigned ch=0;ch<2;++ch) {
    states+=std::bit_cast<std::uint32_t>(LimiterTestAccess::peak(l)[ch])!=s.limiter[ch];
    states+=std::bit_cast<std::uint32_t>(LimiterTestAccess::gain_db(l)[ch])!=s.limiter[ch+2];
    states+=LimiterTestAccess::valid(l)[ch]!=s.cursors[ch*3];states+=s.cursors[ch*3+1]!=145;
    states+=LimiterTestAccess::write(l)[ch]!=s.cursors[ch*3+2];
    for(unsigned i=0;i<145;++i)rings+=std::bit_cast<std::uint32_t>(memory.samples[ch][i])!=s.rings[ch*145+i];
   }
   for(unsigned i=0;i<16;++i)states+=std::bit_cast<std::uint32_t>(MasterTestAccess::gains(master)[i])!=s.gains[i];
  };
  check(initial);const auto count=r.read<std::uint32_t>();if(count!=28)return 2;
  for(unsigned b=0;b<count;++b) {
   const auto dirty=r.read<std::array<std::uint8_t,3>>();const auto reset=r.read<std::uint8_t>();if(reset>1)return 2;for(const auto x:dirty)if(x>1)return 2;
   const auto prepared=snapshot(r);
   const auto input_bits=r.read<std::array<std::uint32_t,32>>(),output_bits=r.read<std::array<std::uint32_t,32>>(),after_input_bits=r.read<std::array<std::uint32_t,32>>();
   const auto after=snapshot(r);
   if(dirty[0])master.set_prepared_pre_gain(std::bit_cast<float>(prepared.controls[0]));
   if(dirty[1])master.set_prepared_post_gain(std::bit_cast<float>(prepared.controls[1]));
   if(dirty[2])master.set_mono(prepared.controls[2]!=0);
   check(prepared);if(reset)master.reset();
   StereoBlock input{},output{};for(unsigned ch=0;ch<2;++ch)for(unsigned i=0;i<16;++i)input[ch][i]=std::bit_cast<float>(input_bits[ch*16+i]);
   if(inplace)master.process(input,input);else master.process(input,output);
   const auto& actual=inplace?input:output;
   for(unsigned ch=0;ch<2;++ch)for(unsigned i=0;i<16;++i) {
    outputs+=std::bit_cast<std::uint32_t>(actual[ch][i])!=output_bits[ch*16+i];
    inputs+=std::bit_cast<std::uint32_t>(input[ch][i])!=after_input_bits[ch*16+i];
   }
   check(after);++blocks;
  }
 }
 r.end();std::cout<<"blocks "<<blocks<<" states "<<states<<" rings "<<rings<<" output "<<outputs<<" input_after "<<inputs<<'\n';
 return states||rings||outputs||inputs?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
