#pragma once
#include <array>
#include <cstdint>
#include <span>
#include <string_view>

namespace mfrk::preset {
using Version = std::array<std::int32_t,3>;
struct Record { Version version; std::span<const std::string_view> labels; };
struct Definition { std::span<const Record> records; std::uint32_t current_item_count; float default_normalized; };
struct Value { bool present; float normalized; };
// Supplied initialized discrete descriptor profile. This does not construct
// descriptors or load a preset, and does not cover custom or blob migration.
Version parse_version(std::string_view text);
Value convert_discrete(const Definition&, Version, Value, bool reset_flag, float reset_normalized);
struct NumericWrite { std::uint32_t id, origin, tag; std::uint64_t length; std::uint32_t bits; };
// Selected tag-1 descriptor, mode 2 and eligible scope only; raw float bits.
NumericWrite select_numeric(std::uint32_t id, bool present, std::uint32_t saved,
                            std::uint32_t fallback, bool reset, std::uint32_t reset_bits);
}
