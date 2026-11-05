#pragma once
#include "mfrk/midi_stream.hpp"
#include <array>
#include <atomic>
#include <cstdint>
#include <span>

namespace mfrk::midi {
struct Message {
 std::int32_t device=0,port=0;
 std::int64_t timestamp=0;
 Event event{};
 bool port_reset=false;
};
// One serialized MIDI producer and one audio consumer. The consumer must call
// recover_overflow before draining each batch and reset all notes/controllers
// if it returns true. No allocation or blocking occurs in either path.
class Ingress {
 struct Source {std::int32_t device=0,port=0;bool active=false;Decoder decoder;};
 struct CallbackContext {Ingress* owner;std::int32_t device,port;std::int64_t timestamp;bool ok;};
 std::array<Source,64> sources_{};
 std::array<Message,4096> queue_{};
 std::atomic<std::uint64_t> read_{0},write_{0};
 std::atomic<bool> overflow_{false};
 bool push(const Message&) noexcept;
 static void emit(void*,const Event&) noexcept;
public:
 static constexpr std::size_t capacity=4096;
 Ingress()=default;
 Ingress(const Ingress&)=delete;Ingress& operator=(const Ingress&)=delete;
 // Timestamp belongs to the input chunk completing an event. Port identity is
 // valid only for the current Android connection and must be reset on removal.
 bool feed(std::int32_t device,std::int32_t port,std::span<const std::uint8_t>,std::int64_t timestamp) noexcept;
 bool reset_port(std::int32_t device,std::int32_t port) noexcept;
 bool pop(Message&) noexcept;
 bool recover_overflow() noexcept;
};
}
