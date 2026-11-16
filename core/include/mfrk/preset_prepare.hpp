#pragma once
#include "mfrk/preset_program.hpp"
#include "mfrk/preset_migration.hpp"
namespace mfrk::preset_prepare {
struct State {std::string creator;std::int32_t word194=-1;preset_program::Numeric numeric;preset_program::Data data;};
struct Versioned {std::string_view name;preset::Definition definition;std::uint32_t flags,reset_bits;};
struct Result {State migrated;bool custom_changed;std::vector<preset_program::Write> writes;};
// Product-66 root/group-0 profile: versioned choices, custom wavetable migration,
// source release stamp, then prepared writes. Metadata word194 is supplied opaque
// state. Other archive/browser metadata must remain preserved by the caller.
// Descriptor construction, bank activation, listeners and DSP delivery are separate.
// Custom blob profile: <=512 bytes, pinned AudioSample archive, ASCII paths.
std::optional<Result> prepare(State,std::span<const preset_program::Descriptor> internal,
 std::span<const preset_program::Descriptor> vst,std::span<const Versioned> versioned);
}
