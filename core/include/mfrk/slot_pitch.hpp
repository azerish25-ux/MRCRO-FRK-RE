#pragma once
#include <array>
#include <cstdint>
#include <cstring>
namespace mfrk::slot_pitch {

// Raw upstream field ABI: nine pitch records and literal dirty/force bytes.
// These are not frontend parameter values or a MIDI/public note setter.
struct Input {float base_note;std::array<std::uint16_t,9> words;std::array<std::uint8_t,9> dirty;std::uint8_t force;};
// Dispatch model pitch only when recomputed is true, even if Hz is unchanged.
// Native model preparation still runs every block in the enclosing slot actor.
struct Result {bool recomputed;float note,hz;};
struct TestAccess;
class Pitch {
 friend struct TestAccess;
 std::array<std::uint8_t,112> state_{};
 struct ZeroTag{};
 explicit Pitch(ZeroTag)noexcept;
 template<class T=std::int32_t>T get(unsigned o)const noexcept{T x;std::memcpy(&x,state_.data()+o,sizeof x);return x;}
 template<class T>void put(unsigned o,T x)noexcept{std::memcpy(state_.data()+o,&x,sizeof x);}
 bool dirty(unsigned i)const noexcept;
 float norm(unsigned i)const noexcept;
 float affine(unsigned i,std::uint32_t width,std::uint32_t minimum)const noexcept;

 void prepare_records()noexcept;
public:
 // Explicit owner preclear plus source PitchComputer constructor stores.
 // Metadata is the recovered fixed shared prototype profile for both sides.
 static Pitch admitted_zero_owner_arena()noexcept;
 // Finite base-note field; all uint16 words/uint8 flags admitted. Rejection
 // preserves state and Result. Nearest rounding/gradual underflow, strict FP.
 bool try_prepare(const Input&in,Result&result)noexcept;
};
}
