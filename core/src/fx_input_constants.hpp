#pragma once
#include <array>
#include <bit>
#include <cstdint>
namespace mfrk::fx_input::detail {
inline constexpr float quarter = std::bit_cast<float>(std::uint32_t{1061752795u});
inline constexpr float three_quarter = std::bit_cast<float>(std::uint32_t{1075235812u});
inline constexpr float calibration = std::bit_cast<float>(std::uint32_t{1041298026u});
inline constexpr std::array<float,7> pattern0{
  std::bit_cast<float>(std::uint32_t{0u}),
  std::bit_cast<float>(std::uint32_t{1065353216u}),
  std::bit_cast<float>(std::uint32_t{3212836864u}),
  std::bit_cast<float>(std::uint32_t{0u}),
  std::bit_cast<float>(std::uint32_t{1056964608u}),
  std::bit_cast<float>(std::uint32_t{3204448256u}),
  std::bit_cast<float>(std::uint32_t{0u}),
};
inline constexpr std::array<float,7> pattern1{
  std::bit_cast<float>(std::uint32_t{3212836864u}),
  std::bit_cast<float>(std::uint32_t{3206125978u}),
  std::bit_cast<float>(std::uint32_t{3192704205u}),
  std::bit_cast<float>(std::uint32_t{1045220557u}),
  std::bit_cast<float>(std::uint32_t{1058642330u}),
  std::bit_cast<float>(std::uint32_t{1065353216u}),
  std::bit_cast<float>(std::uint32_t{0u}),
};
}
