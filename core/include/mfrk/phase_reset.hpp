#pragma once
#include "mfrk/adsr.hpp"
#include "mfrk/gate_dispatch.hpp"
#include <array>
#include <cstdint>
namespace mfrk::phase_reset {
// Raw fields shared by original LFO 2c02f0 and cyclic-phase 2ced10. Caller
// supplies finite values; these names do not imply a complete LFO model.
struct Phase {float reference=0,factor_a=0,factor_b=0,previous=0,current=0;};
void reset(Phase&) noexcept;
// Original 2ceb00 resets its embedded ADSR first, then cyclic phase.
void reset_cyclic(Adsr&,Phase&) noexcept;
// Original shared-voice reset 2cf6f0 writes only these two source fields.
struct Shared {std::uint32_t field18=0,field60=0;};
void reset_shared(Shared&) noexcept;
// The gate-on path also copies source flag 93 to 95, writes 91=0/92=1 and
// performs the phase calculation. Gate-off leaves phase/flags untouched.
struct Cyclic {Adsr* envelope=nullptr;Phase phase;std::uint8_t flag91=0,flag92=0,flag93=0,flag94=0,flag95=0;};
bool gate(Cyclic&,bool on) noexcept;
// Supplied persistent objects; these are reset/gate projections, not complete
// LFO or cyclic-envelope renderers. Indexing is voice*2+side.
struct Bank {
 std::array<Cyclic*,12> cyclic{};
 std::array<Phase*,12> lfo1{},lfo2{};
 std::array<Shared*,6> shared{};
 static bool can_gate(void*,std::uint8_t voice,std::uint8_t side,bool on) noexcept;
 static void gate_callback(void*,std::uint8_t voice,std::uint8_t side,bool on) noexcept;
 static bool can_reset(void*,gate_dispatch::Kind,std::uint8_t voice,std::uint8_t side) noexcept;
 static void reset_callback(void*,gate_dispatch::Kind,std::uint8_t voice,std::uint8_t side) noexcept;
};
}
