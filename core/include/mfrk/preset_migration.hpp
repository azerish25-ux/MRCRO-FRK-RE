#pragma once
#include <array>
#include <cstdint>
#include <span>
#include <optional>
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
struct NumericApplyInput {
 std::uint32_t saved,fallback,reset_value,current,context_word;
 bool present,reset,optimize,dirty;
};
struct NumericNotification {std::uint32_t origin,tag,length,bits;};
struct NumericApplyResult {std::uint32_t bits;bool dirty;std::optional<NumericNotification> notification;};
// Selected saved-in-preset tag-1 descriptor and valid receiver. Preserves the
// original comparison/notification policy, including signed zero and unordered
// values. Registry selection, defaults and downstream delivery are caller-owned.
NumericApplyResult apply_numeric(NumericApplyInput) noexcept;
}
