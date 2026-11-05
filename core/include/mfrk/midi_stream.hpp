#pragma once
#include <array>
#include <cstdint>
#include <span>

namespace mfrk::midi {
// SysEx is delivered as individual ordered bytes including F0/F7; all other
// events contain a status and zero, one, or two data bytes. MIDI 1 byte stream
// only: UMP, timestamp scheduling and device selection are separate layers.
struct Event {
 std::uint8_t status;
 std::array<std::uint8_t,2> data;
 std::uint8_t size;
 bool sysex_byte;
 bool operator==(const Event&) const = default;
};
using Receiver=void(*)(void*,const Event&) noexcept;
class Decoder {
 std::uint8_t running_=0,status_=0,needed_=0,count_=0;
 std::array<std::uint8_t,2> data_{};
 bool sysex_=false;
public:
 void reset() noexcept;
 // Single-owner, allocation-free state; preserve one instance per MIDI port.
 void feed(std::span<const std::uint8_t>,Receiver,void*) noexcept;
};
}
