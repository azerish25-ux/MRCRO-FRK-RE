#include "mfrk/parameter_packet.hpp"
#include <algorithm>
#include <bit>
#include <limits>
namespace mfrk::parameter_packet {
std::optional<Result> scale(std::uint32_t bits,bool is_enum,std::uint32_t count,bool identity,std::span<const std::uint32_t> mapping,std::uint32_t backend_count){
 static_assert(sizeof(float)==4&&std::numeric_limits<float>::is_iec559);
 if(!is_enum)return Result{true,bits};
 if(count>4096||backend_count>4096||mapping.size()>4096||(bits&0x7f800000u)==0x7f800000u)return std::nullopt;
 const float value=std::bit_cast<float>(bits);if(value<-.1f||value>1.1f)return std::nullopt;
 std::uint32_t index=0;
 if(count){const float scaled=static_cast<float>(count)*value;const auto truncated=static_cast<std::int32_t>(scaled);index=static_cast<std::uint32_t>(std::clamp(truncated,0,static_cast<std::int32_t>(count)-1));}
 if(!identity){if(index>=mapping.size())return Result{false,bits};index=mapping[index];}
 const auto target=backend_count?backend_count:count;if(index>=target)return Result{false,bits};
 float result=0;if(target>1)result=static_cast<float>(index)/static_cast<float>(target-1);
 return Result{true,std::bit_cast<std::uint32_t>(result)};
}
}
