#pragma once
#include <array>
#include <bit>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>
namespace mfrk::preset_detail::audio_sample_body {
struct Sample {
    std::array<std::uint8_t,256> raw_path;
    std::uint64_t path_length;
    std::int32_t extra0, extra1;
};
struct Decoded { Sample sample; std::size_t consumed; };
// Native998da0/998f40 BODY profile: owned stream, explicit endian flag,
// raw256-byte buffer, decoded path length0..255, two complete signedint32 words.
// Archive signature/class metadata, path interpretation, custom migration,
// malformed-input exception text and downstream sample loading are separate.
inline std::optional<Decoded> decode(std::span<const std::uint8_t> bytes,bool big=false) {
    if(bytes.size()<256)return std::nullopt;
    Decoded result{};
    for(std::size_t i=0;i<256;++i)result.sample.raw_path[i]=bytes[i];
    std::size_t position=256;
    auto integer=[&](unsigned maximum)->std::optional<std::uint64_t>{
        if(position==bytes.size())return std::nullopt;
        const auto marker=std::bit_cast<std::int8_t>(bytes[position++]);
        const auto count=static_cast<unsigned>(marker<0?-static_cast<int>(marker):marker);
        if(count>maximum||count>bytes.size()-position)return std::nullopt;
        std::uint64_t magnitude=0;
        for(unsigned i=0;i<count;++i){
            const auto index=big?count-1-i:i;
            magnitude|=static_cast<std::uint64_t>(bytes[position+index])<<(8*i);
        }
        position+=count;
        if(marker<0)magnitude=std::uint64_t{0}-magnitude;
        return magnitude;
    };
    const auto length=integer(8),a=integer(4),b=integer(4);
    if(!length||!a||!b||*length>255)return std::nullopt;
    result.sample.path_length=*length;
    result.sample.extra0=std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(*a));
    result.sample.extra1=std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(*b));
    result.consumed=position;
    return result;
}
inline std::vector<std::uint8_t> encode(const Sample& sample,bool big=false) {
    if(sample.path_length>255)throw std::invalid_argument("Unsupported AudioSample length");
    std::vector<std::uint8_t> result(sample.raw_path.begin(),sample.raw_path.end());
    auto integer=[&](std::int64_t value){
        if(value==0){result.push_back(0);return;}
        const bool negative=value<0;
        const auto bits=static_cast<std::uint64_t>(value);
        const auto magnitude=negative?std::uint64_t{0}-bits:bits;
        unsigned count=0;
        for(auto n=magnitude;n;n>>=8)++count;
        const auto marker=static_cast<std::int8_t>(negative?-static_cast<int>(count):static_cast<int>(count));
        result.push_back(std::bit_cast<std::uint8_t>(marker));
        for(unsigned i=0;i<count;++i){
            const auto shift=8*(big?count-1-i:i);
            result.push_back(static_cast<std::uint8_t>(magnitude>>shift));
        }
    };
    integer(static_cast<std::int64_t>(sample.path_length));
    integer(sample.extra0);
    integer(sample.extra1);
    return result;
}
}
