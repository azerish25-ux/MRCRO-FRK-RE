#pragma once
#include <array>
#include <cstdint>
namespace mfrk::notepath::detail {
inline constexpr std::array<std::uint8_t, 108> scale_bytes = {3,253,4,252,5,251,6,250,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,0,0,2,2,4,5,5,7,7,9,9,11,0,0,2,3,3,5,5,7,8,8,10,10,0,0,2,3,3,5,5,7,7,9,10,10,0,0,2,2,4,5,5,7,7,9,10,10,0,0,0,3,3,5,6,7,7,7,10,10,0,0,0,3,3,5,5,7,7,7,10,10,0,1,2,3,4,5,6,7,8,9,10,11};
inline constexpr std::array<std::uint8_t, 17> auxiliary0 = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
inline constexpr std::array<std::uint8_t, 17> auxiliary1 = {0,1,255,2,254,3,253,4,252,5,251,0,0,0,0,0,1};
inline constexpr std::array<std::uint8_t, 17> auxiliary2 = {1,255,2,254,3,253,4,252,5,251,6,250,0,0,0,0,0};
}
