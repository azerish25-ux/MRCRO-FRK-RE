#pragma once
#include "mfrk/karplus.hpp"
namespace mfrk::karplus {struct TestAccess {static const auto& coefficients(const Tables&t){return t.coefficients;}template<class T>static void write(Engine&e,unsigned offset,T value){e.put(offset,value);}static void fill_constructor(Engine&e,std::uint8_t fill){e.state_.fill(fill);e.construct();}static void inputs(Engine&e,const Input&i){e.set_pitch(i.frequency_hz,i.note);e.put(0x28,i.level);if(i.trigger_event==2)e.trigger(true);else if(i.trigger_event==1)e.trigger(false);e.prepare(i);}static const auto&state(const Engine&e){return e.state_;}static const auto&raw(const Engine&e){return e.raw_;}static const auto&excite(const Engine&e){return e.excitation_;}};}

inline float F(std::uint32_t x) noexcept{return std::bit_cast<float>(x);}
inline std::int16_t s16(std::int64_t x) noexcept{return std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(x));}
