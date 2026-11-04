#include "notepath_test_access.hpp"
#include "notepath_allocation_probe.hpp"
#include "notepath_fixture_reader.hpp"
#include <bit>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace mfrk::notepath;
namespace {
std::uint16_t word(const std::uint8_t* p) {
  return std::uint16_t(unsigned(p[0]) | (unsigned(p[1]) << 8));
}
std::int16_t short_word(const std::uint8_t* p) {
  return std::bit_cast<std::int16_t>(word(p));
}
}

unsigned TestAccess::direct(Engine& n, unsigned op, const std::uint8_t* p,
                                   NotePacket& packet, bool& forward,
                                   Publications&) noexcept {
  n.begin_operation();
  if (op == 1) return Engine::free_slot(n.state_.allocator, p[0], p[1]);
  if (op == 2) return Engine::steal_slot(n.state_.allocator, p[0]);
  if (op == 3) {
    const Engine::AllocatorEvent e{p[0], p[1], p[2], p[3], p[4], word(p + 5), p[7]};
    return Engine::allocator_event(n.state_.allocator, e, Engine::on_row, &n);
  }
  if (op == 4) forward = Engine::poly_process(n.state_.poly, packet);
  if (op == 5) forward = Engine::chord_process(n.state_.chord, packet);
  if (op == 6) forward = Engine::unison_process(n.state_.unison, packet);
  if (op == 7) forward = Engine::scale_process(n.state_.scale, packet);
  return 0;
}

int main(int argc, char** argv) try {
  if (argc != 2) throw std::runtime_error("usage: notepath_test original-notepath-fixtures.bin");
  Reader r(argv[1]);
  preflight(r);
  const auto magic_bytes = r.bytes(8);
  const std::string magic(magic_bytes.begin(), magic_bytes.end());
  if (magic != "NPTFX002") throw std::runtime_error("fixture magic");
  if (r.integer(2) != TestAccess::Fields{}.size()) throw std::runtime_error("field schema");
  const auto commands = r.integer(4);
  auto n = Engine::admitted_zero_arena();
  std::array<NoteRow, voice_rows> expected_published{};
  std::uint16_t expected_valid = 0;
  std::uint64_t field_checks = 0, field_bad = 0, packet_checks = 0, packet_bad = 0;
  std::uint64_t row_checks = 0, row_bad = 0, return_bad = 0, forward_checks = 0, forward_bad = 0;
  std::uint64_t hold_checks = 0, hold_bad = 0, published_bad = 0, callback_allocations = 0;
  for (std::uint64_t c = 0; c < commands; ++c) {
    const auto op = static_cast<unsigned>(r.integer(1));
    if (r.integer(1)) {
      TestAccess::load(n, r.fields());
      expected_valid = 0;
    }
    const auto payload = r.bytes(static_cast<std::size_t>(r.integer(2)));
    NotePacket packet;
    if ((op >= 4 && op <= 7) || op == 12) {
      if (payload.size() != packet_bytes) throw std::runtime_error("packet payload size");
      std::copy(payload.begin(), payload.end(), packet.bytes.begin());
    }
    bool forward = false;
    unsigned result = 0;
    HoldDispatch hold{false, 0, 0};
    Publications ignored;
    const auto* p = payload.data();
    notepath_test_allocations::begin();
    if (op == 0) { n = Engine::admitted_zero_arena(); expected_valid = 0; }
    else if (op <= 7) result = TestAccess::direct(n, op, p, packet, forward, ignored);
    else if (op == 8) n.prepare_mode({short_word(p), short_word(p + 2), short_word(p + 4), short_word(p + 6)});
    else if (op == 9) n.prepare_unison_count(short_word(p));
    else if (op == 10) n.prepare_legato(short_word(p));
    else if (op == 11) n.feed_midi_note(p[0] != 0, p[1], p[2]);
    else if (op == 12) n.feed(packet);
    else if (op == 13) {
      const auto value = std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) |
                         (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24);
      hold = n.update_hold_normalized(std::bit_cast<float>(value), p[4] != 0, p[5] != 0);
    } else if (op == 14) n.all_notes_off(p[0] != 0);
    else if (op == 15) n.finish_control_block();
    else throw std::runtime_error("unknown fixture opcode");
    callback_allocations += notepath_test_allocations::end();
    const auto expected = r.fields();
    const auto actual = TestAccess::fields(n);
    for (std::size_t i = 0; i < actual.size(); ++i) {
      ++field_checks;
      if (actual[i] != expected[i]) {
        if (field_bad < 20) std::cerr << "command " << c << " op " << op << " field " << i
                                    << " actual " << actual[i] << " expected " << expected[i] << '\n';
        ++field_bad;
      }
    }
    const auto packet_size = r.integer(1);
    for (unsigned i = 0; i < packet_size; ++i) {
      ++packet_checks;
      packet_bad += packet.bytes[i] != r.integer(1);
    }
    const auto expected_forward = r.integer(1);
    if (expected_forward != 2) { ++forward_checks; forward_bad += forward != bool(expected_forward); }
    return_bad += result != r.varint();
    const auto hold_size = r.integer(1);
    if (hold_size) {
      if (hold_size != 4) throw std::runtime_error("hold packet size");
      hold_checks += 3;
      hold_bad += !hold.processed;
      hold_bad += hold.control_id != r.integer(2);
      hold_bad += std::uint16_t(hold.value) != r.integer(2);
    }
    if (op == 13 && !hold_size) { ++hold_checks; hold_bad += hold.processed; }
    const auto count = r.integer(1);
    const auto& got = n.publications();
    row_bad += got.count != count;
    for (unsigned i = 0; i < count; ++i) {
      const auto row = static_cast<std::uint8_t>(r.integer(1));
      NoteRow original;
      for (auto& b : original.bytes) b = static_cast<std::uint8_t>(r.integer(1));
      ++row_checks;
      if (i >= got.count || got.entries[i].row != row || got.entries[i].value.bytes != original.bytes) ++row_bad;
      expected_published[row] = original;
      expected_valid = std::uint16_t(expected_valid | (1U << row));
    }
    published_bad += n.published_valid_bits() != expected_valid;
    for (unsigned i = 0; i < voice_rows; ++i) {
      if (expected_valid & (1U << i)) published_bad += n.published_rows()[i].bytes != expected_published[i].bytes;
    }
  }
  r.require_end();
  std::cout << "{\"commands\":" << commands << ",\"field_checks\":" << field_checks
            << ",\"field_mismatches\":" << field_bad << ",\"packet_checks\":" << packet_checks
            << ",\"packet_mismatches\":" << packet_bad << ",\"row_checks\":" << row_checks
            << ",\"row_mismatches\":" << row_bad << ",\"return_mismatches\":" << return_bad
            << ",\"forward_checks\":" << forward_checks << ",\"forward_mismatches\":" << forward_bad
            << ",\"hold_checks\":" << hold_checks << ",\"hold_mismatches\":" << hold_bad
            << ",\"published_mismatches\":" << published_bad
            << ",\"callback_allocations\":" << callback_allocations << "}\n";
  return field_bad || packet_bad || row_bad || return_bad || forward_bad || hold_bad || published_bad || callback_allocations ? 1 : 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << '\n';
  return 2;
}
