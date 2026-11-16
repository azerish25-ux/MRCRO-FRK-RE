#pragma once
#include "audio_sample_body_codec.hpp"
#include <algorithm>
namespace mfrk::preset_detail::audio_sample_archive {
// Only known AudioSample fields in the pinned release's native wrapper profile:
// flags0, library10, untracked class-version0, one fresh archive per object.
// This is not generic Boost autodetection. Unknown profiles remain opaque.
inline constexpr std::array<std::uint8_t,29> prefix={
 1,22,'s','e','r','i','a','l','i','z','a','t','i','o','n',':',':','a','r','c','h','i','v','e',1,10,0,0,0};
inline std::optional<audio_sample_body::Decoded> decode(std::span<const std::uint8_t> bytes){
 if(bytes.size()<prefix.size()||!std::equal(prefix.begin(),prefix.end(),bytes.begin()))return std::nullopt;
 auto result=audio_sample_body::decode(bytes.subspan(prefix.size()));
 if(result)result->consumed+=prefix.size();
 return result; // Trailing bytes are not silently discarded; caller gets consumed.
}
inline std::vector<std::uint8_t> encode(const audio_sample_body::Sample& value){
 auto body=audio_sample_body::encode(value);
 std::vector<std::uint8_t> result(prefix.begin(),prefix.end());
 result.insert(result.end(),body.begin(),body.end());
 return result;
}
}
