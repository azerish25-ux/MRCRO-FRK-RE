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
struct Write {std::uint32_t wrapper_id,tag;std::vector<std::uint8_t> payload;bool dirty;std::optional<std::uint64_t> cached_normalized_double_bits;};
// Original group-0 prepared context: internal writes precede positive VST IDs.
// Supplied initialized metadata only; migration and stamping must already have
// run. Unselected state and unknown saved keys remain caller-owned. This stage
// does not activate banks, deliver listeners, schedule work or change DSP state.
std::optional<std::vector<Write>> prepare_group0(std::span<const Descriptor> internal,
 std::span<const Descriptor> vst,const Numeric& saved_numeric,const Data& saved_data);
}
