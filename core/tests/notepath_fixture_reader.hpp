#pragma once
#include "notepath_test_access.hpp"
#include "notepath_field_limits.hpp"
#include <array>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

// Test-only reader for the immutable original fixture. It is not a host parser.
class Reader {
public:
  explicit Reader(const char* path) {
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file) throw std::runtime_error("fixture open failed");
    const auto length=file.tellg();
    if(length<14||length>2*1024*1024) throw std::runtime_error("fixture file size");
    data_.resize(static_cast<std::size_t>(length));file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(data_.data()),length)) throw std::runtime_error("fixture read failed");
  }
  std::uint64_t integer(unsigned size) {
    if(size>8||size>data_.size()-position_) throw std::runtime_error("truncated integer");
    std::uint64_t value=0;
    for(unsigned i=0;i<size;++i)value|=std::uint64_t{data_[position_++]}<<(i*8);
    return value;
  }
  std::uint64_t varint() {
    std::uint64_t value=0;
    for(unsigned i=0;i<10;++i){const auto byte=integer(1);
      if(i==9&&byte>1)throw std::runtime_error("varint overflow");
      value|=(byte&127)<<(i*7);
      if(!(byte&128)){if(i&&byte==0)throw std::runtime_error("noncanonical varint");return value;}
    }
    throw std::runtime_error("varint exceeds ten bytes");
  }
  std::vector<std::uint8_t> bytes(std::size_t count) {
    if(count>data_.size()-position_)throw std::runtime_error("truncated bytes");
    std::vector<std::uint8_t> result(data_.begin()+static_cast<std::ptrdiff_t>(position_),data_.begin()+static_cast<std::ptrdiff_t>(position_+count));position_+=count;return result;
  }
  mfrk::notepath::TestAccess::Fields fields() {
    const auto changes=varint();
    if(changes>previous_.size())throw std::runtime_error("field change count");
    std::size_t index=0;
    for(std::uint64_t i=0;i<changes;++i){const auto delta=varint();
      if(!delta)throw std::runtime_error("nonpositive field delta");
      if(i==0){if(delta>previous_.size())throw std::runtime_error("field index overflow");index=static_cast<std::size_t>(delta-1);}
      else {if(delta>=previous_.size()-index)throw std::runtime_error("field index overflow");index+=static_cast<std::size_t>(delta);}
      const auto value=varint();if(value>field_limits[index])throw std::runtime_error("semantic field width");previous_[index]=value;
    }
    return previous_;
  }
  void require_end() const {if(position_!=data_.size())throw std::runtime_error("trailing fixture data");}
  void require_identity() const {
    std::uint32_t value=0xffffffffu;
    for(auto byte:data_){value^=byte;for(unsigned i=0;i<8;++i)value=(value>>1)^((0u-(value&1u))&0xedb88320u);}
    if(data_.size()!=759036||(value^0xffffffffu)!=1811137614u)throw std::runtime_error("canonical fixture identity");
  }
private:
  std::vector<std::uint8_t> data_;
  std::size_t position_=0;
  mfrk::notepath::TestAccess::Fields previous_{};
};

// Parse framing before executing any actor operation. Length/CRC guards detect
// corruption; verify.py also checks canonical SHA256. Semantic loaded states
// remain trusted original test data, not an arbitrary state-input API.
inline void preflight(Reader r) {
  const auto magic=r.bytes(8);if(std::memcmp(magic.data(),"NPTFX002",8))throw std::runtime_error("fixture magic");
  if(r.integer(2)!=810)throw std::runtime_error("field schema");
  const auto commands=r.integer(4);if(commands!=7172)throw std::runtime_error("fixture command count");
  constexpr std::array<unsigned,16> sizes={0,2,1,8,58,58,58,58,8,2,2,3,58,6,1,0};
  for(std::uint64_t i=0;i<commands;++i){const auto op=r.integer(1),initial=r.integer(1);if(op>=sizes.size()||initial>1)throw std::runtime_error("command framing");if(initial)r.fields();
    const auto size=r.integer(2);if(size!=sizes[op])throw std::runtime_error("command payload size");r.bytes(static_cast<std::size_t>(size));r.fields();
    const auto packet_size=r.integer(1);const bool packet=(op>=4&&op<=7)||op==12;if(packet_size!=(packet?58u:0u))throw std::runtime_error("packet framing");r.bytes(static_cast<std::size_t>(packet_size));
    if(r.integer(1)>2)throw std::runtime_error("forward flag");
    if(r.varint()>255)throw std::runtime_error("return width");
    const auto hold=r.integer(1);if((hold!=0&&hold!=4)||(hold&&op!=13))throw std::runtime_error("hold framing");r.bytes(static_cast<std::size_t>(hold));
    const auto rows=r.integer(1);if(rows>32)throw std::runtime_error("publication count");for(std::uint64_t j=0;j<rows;++j){if(r.integer(1)>=12)throw std::runtime_error("publication row");r.bytes(20);}
  }
  r.require_end();r.require_identity();
}
