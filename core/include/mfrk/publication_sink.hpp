#pragma once
#include "mfrk/notepath.hpp"
#include <array>
#include <cstdint>
namespace mfrk::publication_sink {
struct Owner {
 std::array<notepath::NoteRow,12> rows{};
 std::uint32_t last_active=0,count=0;
 std::array<std::array<std::uint8_t,24>,256> queue{};
};
// Source 2f36b0: copy the row and append a distinct MIDI-formatted record if
// capacity remains. Byte 11 comes from an unwritten native stack byte and is
// explicitly supplied; bytes 12..23 retain their prior contents. This does not
// implement the sequencer queue or downstream voice/audio processing. Input row
// must not alias owner storage. Invalid row/count is rejected without mutation.
bool publish(Owner&,std::uint8_t index,const notepath::NoteRow&,std::uint8_t stack_byte) noexcept;
bool publish_batch(Owner&,const notepath::Publications&,std::uint8_t stack_byte) noexcept;
}
