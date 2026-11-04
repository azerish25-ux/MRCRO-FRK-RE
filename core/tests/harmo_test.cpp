#include "harmo_test_access.hpp"
#include <bit>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <iterator>
#include <stdexcept>
using namespace mfrk::harmo;
static_assert(std::endian::native==std::endian::little);
float F(std::uint32_t bits){return std::bit_cast<float>(bits);}

std::vector<unsigned char> decode(std::vector<unsigned char> packed){
 if(packed.size()>16*1024*1024)throw std::runtime_error("oversize fixture");
 if(packed.size()<8||std::memcmp(packed.data(),"HARMOR01",8))return packed;
 if(packed.size()<16)throw std::runtime_error("truncated packed header");
 auto word=[&](unsigned o){std::uint32_t v;std::memcpy(&v,packed.data()+o,4);return v;};const auto size=word(8),expected_crc=word(12);if(!size||size>16*1024*1024)throw std::runtime_error("decoded bound");std::vector<unsigned char> out;out.reserve(size);
 for(std::size_t p=16;p<packed.size();){const auto tag=packed[p++];const auto n=static_cast<std::size_t>((tag&127)+1);if(n>size-out.size())throw std::runtime_error("decoded overflow");if(tag&128)out.insert(out.end(),n,0);else{if(n>packed.size()-p)throw std::runtime_error("truncated literal");out.insert(out.end(),packed.begin()+p,packed.begin()+p+n);p+=n;}}
 if(out.size()!=size){throw std::runtime_error("decoded size mismatch");}
 std::uint32_t crc=0xffffffffu;for(auto x:out){crc^=x;for(int i=0;i<8;++i)crc=(crc>>1)^((0u-(crc&1u))&0xedb88320u);}if((crc^0xffffffffu)!=expected_crc)throw std::runtime_error("canonical CRC mismatch");return out;
}

int main(int argc,char**argv) try {if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary|std::ios::ate);if(!f)return 2;const auto length=f.tellg();if(length<12||length>16*1024*1024)return 2;std::vector<unsigned char>d(static_cast<std::size_t>(length));f.seekg(0);if(!f.read(reinterpret_cast<char*>(d.data()),length))return 2;d=decode(std::move(d));if(d.size()<12||std::memcmp(d.data(),"HARMO002",8))return 2;std::size_t p=8;auto read=[&]<class T>(){T v{};if(p+sizeof v>d.size())throw std::runtime_error("truncated fixture");std::memcpy(&v,d.data()+p,sizeof v);p+=sizeof v;return v;};const auto cases=read.operator()<std::uint32_t>();if(cases!=13)return 2;Tables tables;std::uint64_t table_errors=0;for(auto x:TestAccess::norm(tables))table_errors+=x!=read.operator()<std::int32_t>();for(auto x:TestAccess::auxiliary(tables))table_errors+=x!=read.operator()<std::uint8_t>();std::uint64_t samples=0,errors=0,prep=0,state=0,delay=0;for(unsigned c=0;c<cases;++c){const bool enabled=read.operator()<std::uint8_t>();const float phase=F(read.operator()<std::uint32_t>());const auto blocks=read.operator()<std::uint32_t>();if(blocks>10000)return 2;auto h=Engine::admitted_zero_arena(tables,enabled);h.set_phase(phase);for(unsigned b=0;b<blocks;++b){Input in{};in.frequency=F(read.operator()<std::uint32_t>());for(auto&x:in.words)x=read.operator()<std::uint16_t>();for(auto&x:in.dirty)x=read.operator()<std::uint8_t>();in.trigger=read.operator()<std::uint8_t>();in.reset_smoothers=read.operator()<std::uint8_t>();in.gain=F(read.operator()<std::uint32_t>());auto full=h;const auto fullout=full.process(in);h.prepare(in);for(auto x:TestAccess::state(h))prep+=x!=read.operator()<std::uint8_t>();auto raw=h.render_q15();for(auto x:TestAccess::state(h))state+=x!=read.operator()<std::uint8_t>();for(auto x:TestAccess::delay(h))delay+=x!=read.operator()<std::int16_t>();for(auto x:raw){auto e=read.operator()<std::int16_t>();errors+=x!=e;}for(auto value:fullout){errors+=std::bit_cast<std::uint32_t>(value)!=read.operator()<std::uint32_t>();++samples;}if(TestAccess::state(full)!=TestAccess::state(h)||TestAccess::delay(full)!=TestAccess::delay(h))return 3;}}std::cout<<"{\"samples\":"<<samples<<",\"table_mismatches\":"<<table_errors<<",\"sample_mismatches\":"<<errors<<",\"prepared_bytes\":"<<prep<<",\"state_bytes\":"<<state<<",\"delay_words\":"<<delay<<"}\n";if(p!=d.size()||samples!=59264)return 2;return errors||prep||state||delay||table_errors;}

catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}
