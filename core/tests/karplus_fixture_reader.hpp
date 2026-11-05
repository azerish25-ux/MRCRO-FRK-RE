#pragma once
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <type_traits>
#include <vector>
class FixtureReader {
 std::vector<std::uint8_t> data_;std::size_t position_=0;
 static constexpr std::size_t max_bytes=32*1024*1024;
 static std::uint32_t word(const std::vector<std::uint8_t>&b,std::size_t p){std::uint32_t x=0;for(unsigned i=0;i<4;++i)x|=std::uint32_t{b[p+i]}<<(8*i);return x;}
public:
 explicit FixtureReader(const char*path){
  static_assert(std::endian::native==std::endian::little);
  std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f)throw std::runtime_error("open");const auto n=f.tellg();if(n<8||n>static_cast<std::streamoff>(max_bytes))throw std::runtime_error("size");std::vector<std::uint8_t> packed(static_cast<std::size_t>(n));f.seekg(0);if(!f.read(reinterpret_cast<char*>(packed.data()),n))throw std::runtime_error("read");
  if(std::memcmp(packed.data(),"KRZP0001",8)!=0){data_=std::move(packed);return;}
  if(packed.size()<16){throw std::runtime_error("packet header");}const auto size=word(packed,8),crc_expected=word(packed,12);if(size<8||size>max_bytes)throw std::runtime_error("decoded size");data_.reserve(size);
  for(std::size_t p=16;p<packed.size();){const auto tag=packed[p++];const auto count=std::size_t{(tag&127u)+1};if(count>size-data_.size())throw std::runtime_error("decoded overflow");if(tag&128u)data_.insert(data_.end(),count,0);else{if(count>packed.size()-p)throw std::runtime_error("literal truncated");data_.insert(data_.end(),packed.begin()+static_cast<std::ptrdiff_t>(p),packed.begin()+static_cast<std::ptrdiff_t>(p+count));p+=count;}}
  if(data_.size()!=size){throw std::runtime_error("decoded length");}std::uint32_t crc=0xffffffffu;for(const auto x:data_){crc^=x;for(unsigned bit=0;bit<8;++bit)crc=(crc>>1)^((0u-(crc&1u))&0xedb88320u);}if((crc^0xffffffffu)!=crc_expected)throw std::runtime_error("CRC32");
 }
 template<class T>T read(){static_assert(std::is_trivially_copyable_v<T>);T x;if(sizeof x>data_.size()-position_)throw std::runtime_error("truncated");std::memcpy(&x,data_.data()+position_,sizeof x);position_+=sizeof x;return x;}
 void end()const{if(position_!=data_.size())throw std::runtime_error("trailing");}
};

