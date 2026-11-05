#pragma once
#include "mfrk/midi_stream.hpp"
#include "mfrk/notepath.hpp"
namespace mfrk::midi {
enum class NoteResult { delivered, filtered, unsupported, invalid };
using PublicationSink=void(*)(void*,const notepath::Publications&) noexcept;
// Channel 0..15 filters that channel; 16 is explicit omni. Each delivered note
// must synchronously hand its publication batch to the owner before any next
// NotePath operation without allocation, blocking or reentry. CC, pitch bend,
// MPE, transport and port reset are separate.
NoteResult route_note(const Event&,unsigned channel,notepath::Engine&,PublicationSink,void*) noexcept;
}
