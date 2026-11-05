#pragma once
#include "mfrk/adsr.hpp"
#include "mfrk/core_bank.hpp"
#include "mfrk/gate_dispatch.hpp"
#include <array>
namespace mfrk::voice_gate {
// Cyclic callback must implement both the envelope gate and original cyclic
// phase/reset side effects; it runs synchronously on the owner thread.
using Cyclic=void(*)(void*,std::uint8_t voice,std::uint8_t side,bool on) noexcept;
struct Owner {
 // Indexed voice*2+side. Objects are persistent and owned by the caller.
 std::array<core_bank::SlotController*,12> slots{};
 std::array<Adsr*,12> main_envelopes{};
 Cyclic cyclic=nullptr;
 void* cyclic_context=nullptr;
};
// Applies the bounded gate projection atomically with respect to rejected
// inputs. Pitch, LFO, row processing and remaining voice side effects are not
// performed here. Caller owns valid object lifetimes and single-thread access.
bool dispatch(Owner&,gate_dispatch::State&,std::uint8_t row,std::uint8_t status,
              bool para,std::uint8_t cyclic_mode,bool cyclic_disabled) noexcept;
}
