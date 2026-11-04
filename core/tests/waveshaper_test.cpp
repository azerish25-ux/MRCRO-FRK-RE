#include "waveshaper_allocation_probe.hpp"
#include "waveshaper_fixture_reader.hpp"
#include <cmath>
#include "waveshaper_test_access.hpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mfrk::waveshaper;

void preflight(Reader r){
 if(r.read<std::array<char,8>>()!=std::array<char,8>{'W','S','H','P','0','0','0','1'})throw std::runtime_error("magic");
 if(r.read<std::uint32_t>()!=4)throw std::runtime_error("case count");
 r.read<std::array<std::uint8_t,6682>>();
 if(r.read<std::uint32_t>()!=4)throw std::runtime_error("constructor count");
 for(int i=0;i<4;++i){const auto fill=r.read<std::uint8_t>();if((fill!=0&&fill!=165)||r.read<std::uint8_t>()>1)throw std::runtime_error("constructor tag");r.read<std::array<std::uint8_t,432>>();const auto mask=r.read<std::array<std::uint8_t,216>>();unsigned written=0;for(auto x:mask){if(x>1)throw std::runtime_error("constructor mask");written+=x;}if(written!=52)throw std::runtime_error("constructor mask count");}
 unsigned total=0;
 for(int c=0;c<4;++c){if(r.read<std::uint8_t>()>1||r.read<std::uint8_t>()!=1)throw std::runtime_error("case tag");const auto blocks=r.read<std::uint32_t>();if(!blocks||blocks>10000)throw std::runtime_error("block count");total+=blocks;
  for(unsigned b=0;b<blocks;++b){r.read<std::uint32_t>();r.read<std::array<std::uint16_t,3>>();r.read<std::array<std::uint8_t,3>>();if(r.read<std::uint8_t>()>15)throw std::runtime_error("flags");const auto level=F(r.read<std::uint32_t>());if(!std::isfinite(level))throw std::runtime_error("level");r.read<std::array<std::uint8_t,816>>();}
 }
 if(total!=1612)throw std::runtime_error("total blocks");
 r.end();r.identity(1353730,1146221373u);
}

int main(int argc,char**argv)try{if(argc!=2)return 2;Reader r(argv[1]);preflight(r);if(r.read<std::array<char,8>>()!=std::array<char,8>{'W','S','H','P','0','0','0','1'})return 2;auto cases=r.read<std::uint32_t>();if(!cases||cases>100)return 2;Tables tables;unsigned table_errors=0,ctor_errors=0,prep_errors=0,state_errors=0,slope_errors=0,fold_errors=0,raw_errors=0,scaled_errors=0;std::uint64_t samples=0,blocks=0,callback_allocations=0;
 for(const auto&array:{TestAccess::coarse(tables),TestAccess::fine(tables)}){for(float x:array){table_errors+=std::bit_cast<std::uint32_t>(x)!=r.read<std::uint32_t>();}}
 for(float x:TestAccess::table_fold(tables))table_errors+=std::bit_cast<std::uint32_t>(x)!=r.read<std::uint32_t>();
 for(const auto&row:TestAccess::shape(tables))for(auto x:row)table_errors+=x!=r.read<std::int16_t>();
 auto ctors=r.read<std::uint32_t>();if(ctors!=4)return 2;
 for(unsigned c=0;c<ctors;++c){auto fill=r.read<std::uint8_t>();if(r.read<std::uint8_t>()>1)return 2;auto e=Engine::admitted_zero_arena(tables);TestAccess::fill_constructor(e,fill);for(auto x:TestAccess::state(e))ctor_errors+=x!=r.read<std::uint8_t>();e.initialize();for(auto x:TestAccess::state(e))ctor_errors+=x!=r.read<std::uint8_t>();r.read<std::array<std::uint8_t,216>>();}
 for(unsigned c=0;c<cases;++c){if(r.read<std::uint8_t>()>1)return 2;const auto initialized=r.read<std::uint8_t>();if(initialized>1)return 2;const auto count=r.read<std::uint32_t>();if(!count||count>10000)return 2;auto e=Engine::admitted_zero_arena(tables);if(initialized)e.initialize();
  for(unsigned b=0;b<count;++b){Input in;in.pitch_argument=std::bit_cast<float>(r.read<std::uint32_t>());in.words=r.read<std::array<std::uint16_t,3>>();in.dirty=r.read<std::array<std::uint8_t,3>>();const auto flags=r.read<std::uint8_t>();if(flags>15)return 2;in.gate=flags&1;in.initialize=flags&2;in.reset=flags&4;in.interface_prepare=flags&8;in.output_level=std::bit_cast<float>(r.read<std::uint32_t>());auto full=e;std::array<float,32> scaled{};waveshaper_test_allocations::begin();if(full.try_process(in,scaled)!=RenderStatus::ok)throw std::runtime_error("process admission");TestAccess::prepare(e,in);for(auto x:TestAccess::state(e))prep_errors+=x!=r.read<std::uint8_t>();std::array<float,32> raw{};if(e.try_render_raw(raw)!=RenderStatus::ok)throw std::runtime_error("raw admission");for(auto x:TestAccess::state(e))state_errors+=x!=r.read<std::uint8_t>();for(auto x:TestAccess::slope(e))slope_errors+=std::bit_cast<std::uint32_t>(x)!=r.read<std::uint32_t>();for(auto x:TestAccess::fold(e))fold_errors+=std::bit_cast<std::uint32_t>(x)!=r.read<std::uint32_t>();for(auto x:raw)raw_errors+=std::bit_cast<std::uint32_t>(x)!=r.read<std::uint32_t>();for(auto x:scaled)scaled_errors+=std::bit_cast<std::uint32_t>(x)!=r.read<std::uint32_t>();callback_allocations+=waveshaper_test_allocations::end();if(TestAccess::state(e)!=TestAccess::state(full))throw std::runtime_error("publicprocess state");samples+=32;++blocks;}
 }
 r.end();std::cout<<"{\"blocks\":"<<blocks<<",\"samples\":"<<samples<<",\"tables\":"<<table_errors<<",\"constructor\":"<<ctor_errors<<",\"prepared\":"<<prep_errors<<",\"state\":"<<state_errors<<",\"slope\":"<<slope_errors<<",\"fold\":"<<fold_errors<<",\"raw\":"<<raw_errors<<",\"scaled\":"<<scaled_errors<<",\"callback_allocations\":"<<callback_allocations<<"}\n";return table_errors||ctor_errors||prep_errors||state_errors||slope_errors||fold_errors||raw_errors||scaled_errors||callback_allocations?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 2;}
