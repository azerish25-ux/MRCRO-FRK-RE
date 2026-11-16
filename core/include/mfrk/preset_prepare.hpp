#pragma once
#include "mfrk/preset_program.hpp"
#include "mfrk/preset_migration.hpp"
#include "mfrk/preset_archive.hpp"
namespace mfrk::preset_prepare {
struct State {std::string creator;std::int32_t word194=-1;preset_program::Numeric numeric;preset_program::Data data;};
struct Versioned {std::string_view name;preset::Definition definition;std::uint32_t flags,reset_bits;};
struct Result {State migrated;bool custom_changed;std::vector<preset_program::Write> writes;};
struct Loaded {preset_archive::Owned archive;Result prepared;};
// Copies the product-66 storage view into an owning, unmigrated program state.
// The opaque native word is explicit; browser metadata and raw bytes stay with
// the caller. No descriptor lookup, stamp or DSP mutation occurs here.
std::optional<State> from_archive(const preset_archive::View&,std::int32_t word194);
// Source serializer binding for the installed class-7 library profile:
// HardwareVersion absent -> -1, canonical "49" -> 49. Other property values
// are outside the captured profile and require the explicit-word overload.
std::optional<State> from_archive(const preset_archive::View&);
// Product-66 root/group-0 profile: versioned choices, custom wavetable migration,
// source release stamp, then prepared writes. Metadata word194 is supplied opaque
// state. Other archive/browser metadata must remain preserved by the caller.
// Descriptor construction, bank activation, listeners and DSP delivery are separate.
// Custom blob profile: <=512 bytes, pinned AudioSample archive, ASCII paths.
std::optional<Result> prepare(State,std::span<const preset_program::Descriptor> internal,
 std::span<const preset_program::Descriptor> vst,std::span<const Versioned> versioned);
// Owning staging boundary for asynchronous loading. Retains the exact archive
// and browser metadata beside the migrated program; does not activate a bank.
std::optional<Loaded> load(preset_archive::Owned,std::int32_t word194,
 std::span<const preset_program::Descriptor> internal,
 std::span<const preset_program::Descriptor> vst,std::span<const Versioned> versioned);
std::optional<Loaded> load(preset_archive::Owned,
 std::span<const preset_program::Descriptor> internal,
 std::span<const preset_program::Descriptor> vst,std::span<const Versioned> versioned);
}
