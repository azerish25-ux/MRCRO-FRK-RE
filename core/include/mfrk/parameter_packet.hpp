#pragma once
#include <cstdint>
#include <optional>
#include <span>
namespace mfrk::parameter_packet {
struct Result {bool accepted;std::uint32_t bits;};
// Source 75da30 current frontend-to-processor normalized scalar conversion.
// Separate from archived-version migration and physical DSP units. Continuous
// values preserve every bit. Enum profile: finite [-.1f,1.1f], counts/map <=4096,
// default round-to-nearest arithmetic. An unaccepted result preserves input.
// Nullopt denotes inputs outside this bounded profile, not a native rejection.
std::optional<Result> scale(std::uint32_t bits,bool is_enum,std::uint32_t count,
 bool identity,std::span<const std::uint32_t> mapping,std::uint32_t backend_count);
}
