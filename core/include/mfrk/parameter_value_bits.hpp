#pragma once
#include <bit>
#include <cstdint>
namespace mfrk::parameter_value_bits {
// Source UCOMISS/JE equality, with FP exception status excluded.
inline bool equal_scalar(std::uint32_t a,std::uint32_t b) noexcept {
 if((a&0x7fffffffu)>0x7f800000u||(b&0x7fffffffu)>0x7f800000u)return true;
 if((a&0x7fffffffu)==0&&(b&0x7fffffffu)==0)return true;
 return a==b;
}
// Source CVTSS2SD bit result under MXCSR1f80, including signed zero and quieted
// NaN payloads. Integer construction avoids host FP-status and NaN differences.
inline std::uint64_t promote_scalar(std::uint32_t bits) noexcept {
 const auto sign=std::uint64_t(bits&0x80000000u)<<32;
 const auto exponent=(bits>>23)&255u,fraction=bits&0x7fffffu;
 if(exponent==255)return sign|0x7ff0000000000000ull|(std::uint64_t(fraction)<<29)|(fraction?0x0008000000000000ull:0);
 if(exponent)return sign|(std::uint64_t(exponent+896)<<52)|(std::uint64_t(fraction)<<29);
 if(!fraction)return sign;
 const auto top=31-std::countl_zero(fraction);
 return sign|(std::uint64_t(top+874)<<52)|(std::uint64_t(fraction^(1u<<top))<<(52-top));
}
// Source 64a930 -> 64b030. Origins 0/1 store promoted raw bits without a changed
// event; other unsigned origins clamp via MAXSD/MINSD then UCOMISD/JE. An old
// NaN cache is retained, and equal signed-zero caches retain their prior sign.
// Returns whether the owner changed(message=1) callback must be invoked.
inline bool update_cache(std::uint64_t& cache,std::uint32_t bits,std::uint32_t origin) noexcept {
 if(origin<2){cache=promote_scalar(bits);return false;}
 std::uint64_t next=0;
 if((bits&0x80000000u)==0&&(bits&0x7fffffffu)<=0x7f800000u)next=bits>=0x3f800000u?0x3ff0000000000000ull:promote_scalar(bits);
 const auto magnitude=cache&0x7fffffffffffffffull;
 if(magnitude>0x7ff0000000000000ull||cache==next||(magnitude==0&&next==0))return false;
 cache=next;return true;
}
}
