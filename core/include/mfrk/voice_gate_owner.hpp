#pragma once
#include "mfrk/adsr.hpp"
#include "mfrk/core_bank.hpp"
#include "mfrk/gate_dispatch.hpp"
#include <array>
namespace mfrk::voice_gate {
// Cyclic callback must implement both the envelope gate and original cyclic
// phase/reset side effects; it runs synchronously on the owner thread without
// blocking, allocating or reentering the gate owner.
using Cyclic=void(*)(void*,std::uint8_t voice,std::uint8_t side,bool on) noexcept;
using CyclicReady=bool(*)(void*,std::uint8_t voice,std::uint8_t side,bool on) noexcept;
// Handles original shared-voice, cyclic and LFO reset actions without allocation or reentry.
using Reset=void(*)(void*,gate_dispatch::Kind,std::uint8_t voice,std::uint8_t side) noexcept;
using ResetReady=bool(*)(void*,gate_dispatch::Kind,std::uint8_t voice,std::uint8_t side) noexcept;
struct Owner {
 // Indexed voice*2+side. Objects are persistent and owned by the caller.
 std::array<core_bank::SlotController*,12> slots{};
 std::array<Adsr*,12> main_envelopes{};
 Cyclic cyclic=nullptr;
 CyclicReady cyclic_ready=nullptr;
 void* cyclic_context=nullptr;
 Reset reset=nullptr;
 ResetReady reset_ready=nullptr;
 void* reset_context=nullptr;
};
// Applies the bounded gate projection atomically with respect to rejected
// inputs. Pitch, LFO, row processing and remaining voice side effects are not
// performed here. Caller owns valid object lifetimes and single-thread access.
bool dispatch(Owner&,gate_dispatch::State&,std::uint8_t row,std::uint8_t status,
              bool para,std::uint8_t cyclic_mode,bool cyclic_disabled,const gate_dispatch::ResetContext* reset_context=nullptr) noexcept;
// Gate projection for one published-row snapshot, in native voice/side order.
// Does not clear row statuses; the owner does so after the full voice stage.
bool process_rows(Owner&,gate_dispatch::State&,const std::array<notepath::NoteRow,12>&,
                  bool para,std::uint8_t cyclic_mode,bool cyclic_disabled,const std::uint16_t* row_mask=nullptr) noexcept;
}
