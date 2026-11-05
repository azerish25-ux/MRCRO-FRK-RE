#pragma once
#include "ui_scene_fixture_identity.hpp"
#include <bit>
#include <cstring>
#include <fstream>
#include <span>
#include <stdexcept>
#include <vector>
namespace mfrk::ui::scene::test {
inline std::uint32_t crc32(std::span<const std::uint8_t> bytes) noexcept {
 std::uint32_t crc=0xffffffffu;for(auto byte:bytes){crc^=byte;for(unsigned j=0;j<8;++j)crc=(crc>>1)^(0xedb88320u&(0u-(crc&1u)));}return crc^0xffffffffu;
}
inline std::array<std::uint8_t,32> sha256(std::span<const std::uint8_t> bytes){
 constexpr std::array<std::uint32_t,64> k{
 0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
 0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
 0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
 0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u};
 std::array<std::uint32_t,8> h{0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u};
 std::vector<std::uint8_t> message(bytes.begin(),bytes.end());const auto bit_length=std::uint64_t(bytes.size())*8u;message.push_back(0x80);while(message.size()%64!=56)message.push_back(0);for(int i=7;i>=0;--i)message.push_back(static_cast<std::uint8_t>(bit_length>>(i*8)));
 for(std::size_t base=0;base<message.size();base+=64){std::array<std::uint32_t,64>w{};for(unsigned j=0;j<16;++j)for(unsigned b=0;b<4;++b)w[j]=(w[j]<<8)|message[base+j*4+b];for(unsigned j=16;j<64;++j){const auto x=w[j-15],y=w[j-2];w[j]=w[j-16]+(std::rotr(x,7)^std::rotr(x,18)^(x>>3))+w[j-7]+(std::rotr(y,17)^std::rotr(y,19)^(y>>10));}auto state=h;for(unsigned j=0;j<64;++j){const auto a=state[0],b=state[1],c=state[2],d=state[3],e=state[4],f=state[5],g=state[6],hh=state[7];const auto t1=hh+(std::rotr(e,6)^std::rotr(e,11)^std::rotr(e,25))+((e&f)^(~e&g))+k[j]+w[j];const auto t2=(std::rotr(a,2)^std::rotr(a,13)^std::rotr(a,22))+((a&b)^(a&c)^(b&c));state={t1+t2,a,b,c,d+t1,e,f,g};}for(unsigned j=0;j<8;++j)h[j]+=state[j];}
 std::array<std::uint8_t,32> result{};for(unsigned j=0;j<8;++j)for(unsigned b=0;b<4;++b)result[j*4+b]=static_cast<std::uint8_t>(h[j]>>(24-b*8));return result;
}
inline const Identity& identity(std::string_view kind){for(const auto& value:identities)if(value.kind==kind)return value;throw std::runtime_error("unknown fixture kind");}
class Reader {
 std::vector<std::uint8_t>data_;std::size_t position_=0;
 void need(std::size_t count)const{if(position_>data_.size()||count>data_.size()-position_)throw std::runtime_error("truncated fixture");}
public:
 // Authenticate complete raw bytes BEFORE interpreting any numeric payload.
 Reader(std::vector<std::uint8_t>data,const Identity& expected):data_(std::move(data)){
  if(data_.size()!=expected.size)throw std::runtime_error("canonical fixture size");
  if(crc32(data_)!=expected.crc)throw std::runtime_error("canonical fixture CRC32");
  if(sha256(data_)!=expected.sha)throw std::runtime_error("canonical fixture SHA256");
  need(12);if(std::memcmp(data_.data(),expected.magic.data(),8))throw std::runtime_error("fixture magic");position_=8;if(u32()!=expected.count)throw std::runtime_error("fixture count");
 }
 static Reader file(const char*path,const Identity& expected){std::ifstream input(path,std::ios::binary|std::ios::ate);if(!input)throw std::runtime_error("fixture open");const auto length=input.tellg();if(length<0||static_cast<std::uint64_t>(length)!=expected.size||expected.size>8*1024*1024)throw std::runtime_error("canonical fixture file size");std::vector<std::uint8_t>data(expected.size);input.seekg(0);if(!input.read(reinterpret_cast<char*>(data.data()),static_cast<std::streamsize>(data.size())))throw std::runtime_error("fixture read");return Reader(std::move(data),expected);}
 void zeroes(std::size_t n){need(n);for(std::size_t j=0;j<n;++j)if(data_[position_++]!=0)throw std::runtime_error("unsupported fixture native pointer/padding");}
 std::uint8_t u8(){need(1);return data_[position_++];}
 bool flag(){const auto v=u32();if(v>1)throw std::runtime_error("fixture boolean");return v!=0;}
 std::uint32_t u32(){need(4);std::uint32_t v=0;for(unsigned j=0;j<4;++j)v|=std::uint32_t(data_[position_++])<<(j*8);return v;}
 std::int32_t i32(){return std::bit_cast<std::int32_t>(u32());}
 float f32(){return std::bit_cast<float>(u32());}
 double f64(){std::uint64_t v=u32();v|=std::uint64_t(u32())<<32;return std::bit_cast<double>(v);}
 void end()const{if(position_!=data_.size())throw std::runtime_error("trailing fixture data");}
};
} // namespace mfrk::ui::scene::test
