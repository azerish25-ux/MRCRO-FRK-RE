#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <utility>

// Trusted-reference reader, not a host parser. Bound framing and CRC checks
// happen before engine operations; the verification script also checks SHA256.
class Reader {
    std::vector<std::uint8_t> data_;
    std::size_t position_=0;
    static std::uint32_t word(const std::vector<std::uint8_t>& data,std::size_t offset) {
        std::uint32_t value;std::memcpy(&value,data.data()+offset,4);return value;
    }
    static std::uint32_t crc32(const std::vector<std::uint8_t>& data) noexcept {
        std::uint32_t crc=0xffffffffu;
        for(auto byte:data){crc^=byte;for(unsigned i=0;i<8;++i)crc=(crc>>1)^((0u-(crc&1u))&0xedb88320u);}
        return crc^0xffffffffu;
    }
    static std::vector<std::uint8_t> decode(std::vector<std::uint8_t> input) {
        if(input.size()<8||std::memcmp(input.data(),"WSPK0001",8))return input;
        if(input.size()<16)throw std::runtime_error("packed header");
        const auto size=word(input,8),expected_crc=word(input,12);
        if(!size||size>16*1024*1024)throw std::runtime_error("packed decoded size");
        std::vector<std::uint8_t> output;output.reserve(size);
        for(std::size_t p=16;p<input.size();){
            const auto tag=input[p++];const auto count=std::size_t{(tag&127u)+1};
            if(count>size-output.size())throw std::runtime_error("packed overflow");
            if(tag&128u)output.insert(output.end(),count,0);
            else {
                if(count>input.size()-p)throw std::runtime_error("packed literal");
                output.insert(output.end(),input.begin()+static_cast<std::ptrdiff_t>(p),input.begin()+static_cast<std::ptrdiff_t>(p+count));p+=count;
            }
        }
        if(output.size()!=size)throw std::runtime_error("packed length");
        if(crc32(output)!=expected_crc)throw std::runtime_error("packed CRC");
        return output;
    }
public:
    explicit Reader(const char* path) {
        std::ifstream file(path,std::ios::binary|std::ios::ate);
        if(!file)throw std::runtime_error("open");
        const auto length=file.tellg();
        if(length<12||length>16*1024*1024)throw std::runtime_error("file size");
        std::vector<std::uint8_t> input(static_cast<std::size_t>(length));file.seekg(0);
        if(!file.read(reinterpret_cast<char*>(input.data()),length))throw std::runtime_error("read");
        data_=decode(std::move(input));
    }
    template<class T> T read() {
        static_assert(std::is_trivially_copyable_v<T>);
        T value;
        if(sizeof value>data_.size()-position_)throw std::runtime_error("truncated");
        std::memcpy(&value,data_.data()+position_,sizeof value);position_+=sizeof value;return value;
    }
    void identity(std::size_t expected_bytes,std::uint32_t expected_crc) const {
        if(data_.size()!=expected_bytes||crc32(data_)!=expected_crc)throw std::runtime_error("fixture identity");
    }
    void end() const {if(position_!=data_.size())throw std::runtime_error("trailing");}
};
