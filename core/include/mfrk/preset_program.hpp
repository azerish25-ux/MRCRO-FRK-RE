#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
namespace mfrk::preset_program {
using Numeric=std::map<std::string,std::uint32_t,std::less<>>;
using Data=std::map<std::string,std::vector<std::uint8_t>,std::less<>>;
struct Descriptor {std::uint32_t flags,reset_bits,default_tag;std::span<const std::uint8_t> fallback;std::string_view name{};};
// cached_normalized_double_bits projects the fresh +0-cache reference
// profile. Persistent receivers apply their own cache history; this field must
// never overwrite an owner's cache during staging or activation.
struct Write {std::uint32_t wrapper_id,tag;std::vector<std::uint8_t> payload;bool dirty;std::optional<std::uint64_t> cached_normalized_double_bits;};
struct Command {std::uint32_t wrapper_id,tag,origin;std::vector<std::uint8_t> payload;};
// Original group-0 prepared context: internal writes precede positive VST IDs.
// Supplied initialized metadata only; migration and stamping must already have
// run. Unselected state and unknown saved keys remain caller-owned. This stage
// does not activate banks, deliver listeners, schedule work or change DSP state.
std::optional<std::vector<Write>> prepare_group0(std::span<const Descriptor> internal,
 std::span<const Descriptor> vst,const Numeric& saved_numeric,const Data& saved_data);
// Source group-0 ordinary-origin notification traversal, AFTER bank activation.
// The caller supplies current bank values and current descriptor flags. This
// creates owned commands only; activation, listener dispatch and DSP are separate.
std::optional<std::vector<Command>> notifications_group0(std::span<const std::uint32_t> internal_flags,
 std::span<const std::uint32_t> vst_flags,std::span<const Write> current);
}
