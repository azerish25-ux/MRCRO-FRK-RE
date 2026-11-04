#pragma once
#include "mfrk/waveshaper.hpp"
namespace mfrk::waveshaper {
inline float F(std::uint32_t x) noexcept {return std::bit_cast<float>(x);}
struct TestAccess {
 static const auto& coarse(const Tables&t) noexcept {return t.coarse;}
 static const auto& fine(const Tables&t) noexcept {return t.fine;}
 static const auto& table_fold(const Tables&t) noexcept {return t.fold;}
 static const auto& shape(const Tables&t) noexcept {return t.shape;}
 static float frequency(const Tables&t,float note) noexcept {return t.frequency(note);}
 static std::int32_t conversion(float x) noexcept {return Engine::cvtt32(x);}
 static void load(Engine&e,const std::array<std::uint8_t,216>& bytes) noexcept {e.state_=bytes;e.initialized_=true;}
 static const auto& state(const Engine&e) noexcept{return e.state_;}
 static const auto& slope(const Engine&e) noexcept{return e.slope_;}
 static const auto& fold(const Engine&e) noexcept{return e.folded_;}
 static void fill_constructor(Engine&e,std::uint8_t fill) noexcept {e.state_.fill(fill);e.construct();}
 static void prepare(Engine&e,const Input&in) noexcept {e.set_controls(in.words,in.dirty);e.put(0x28,in.output_level);e.set_pitch(12345.0f,in.pitch_argument);e.trigger(in.gate);if(in.initialize)e.initialize();if(in.reset)e.reset();e.prepare(in.interface_prepare);}
};
}
