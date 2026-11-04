#include "mfrk/notepath.hpp"
#include "notepath_source_tables.hpp"
#include <algorithm>
#include <bit>
#include <cstddef>
#include <limits>

namespace mfrk::notepath {
namespace {
std::uint8_t byte(int v) noexcept { return static_cast<std::uint8_t>(v); }
std::uint16_t word(unsigned v) noexcept { return static_cast<std::uint16_t>(v); }
int signed_byte(std::uint8_t v) noexcept { return std::bit_cast<std::int8_t>(v); }
void put16(NoteRow& r, unsigned i, std::uint16_t v) noexcept {
  r.bytes[i] = byte(v); r.bytes[i + 1] = byte(v >> 8);
}
std::uint32_t get32(const NoteRow& r, unsigned i) noexcept {
  return std::uint32_t(r.bytes[i]) | (std::uint32_t(r.bytes[i + 1]) << 8) |
         (std::uint32_t(r.bytes[i + 2]) << 16) | (std::uint32_t(r.bytes[i + 3]) << 24);
}
void put32(NoteRow& r, unsigned i, std::uint32_t v) noexcept {
  for (unsigned j = 0; j < 4; ++j) r.bytes[i + j] = byte(static_cast<int>((v >> (8 * j)) & 255));
}
void append(NotePacket& p, std::uint8_t pitch, std::uint8_t velocity, std::uint8_t flags) noexcept {
  const unsigned n = p.bytes[0x35];
  p.bytes[4 + n * 3] = pitch; p.bytes[5 + n * 3] = velocity;
  p.bytes[6 + n * 3] = flags; p.bytes[0x35] = byte(static_cast<int>(n + 1));
}
int discrete(std::int16_t value, int count) noexcept {
  return (int(value) * count + (value < 0 ? -1 : 1)) / 32768;
}
}

void Engine::advance_age(AllocatorState& s, unsigned row) noexcept {
  ++s.age_counter; put32(s.rows[row], 12, s.age_counter);
}

std::uint8_t Engine::choose_note(const PolyState& s) noexcept {
  unsigned selected = 128;
  for (unsigned i = 0; i < 128; ++i) {
    if (!s.age[i]) continue;
    if (selected == 128) { selected = i; if (s.priority == 0) break; }
    else if (s.priority == 1 || (s.priority == 2 && s.age[i] > s.age[selected]) ||
             (s.priority == 3 && s.age[i] < s.age[selected])) selected = i;
  }
  return byte(static_cast<int>(selected));
}

NotePacket Engine::empty_packet() noexcept {
  NotePacket p;
  p.bytes[1] = 128;
  for (unsigned i = 0; i < 16; ++i) p.bytes[4 + i * 3] = 128;
  p.bytes[0x38] = 128;
  return p;
}
Engine::Engine(AdmittedZeroArenaTag) noexcept {
  state_.poly.velocity.fill(100);
  state_.poly.pending = empty_packet(); state_.last_input = empty_packet();
  for (auto& row : state_.allocator.rows) {
    row.bytes[1] = 60; row.bytes[3] = 100; put16(row, 4, 16000); row.bytes[18] = 60;
  }
}
void Engine::reset_poly(PolyState& p) noexcept {
  p.age.fill(0); p.velocity.fill(100); p.selected = 128; p.source = 0;
  // Counter, configuration and pending output packet are retained by32dc30.
}
bool Engine::poly_process(PolyState& s, NotePacket& packet) noexcept {
  auto& p = packet.bytes;
  if (!s.enabled) return true;
  if (p[0x34]) return false;
  if (p[3] == 2 && s.source == 1) return false;
  if (p[3] == 1 && s.source == 2) {
    if (s.selected < 128) {
      s.pending = empty_packet(); s.pending.bytes[0] = 1;
      s.pending.bytes[1] = s.selected; s.pending.bytes[3] = s.source;
    }
    reset_poly(s);
  }
  if (s.source != 0 || p[0] != 1) s.source = p[3];
  const auto old = s.selected;
  const unsigned count = p[0x35] < 128 ? p[0x35] : 0;
  if (p[0] == 0) {
    const auto age = s.counter ? s.counter : std::uint16_t{1};
    s.counter = word(unsigned(age) + 1); s.age[p[1]] = age; s.velocity[p[1]] = p[2];
    s.selected = choose_note(s);
    for (unsigned i = 0; i < count; ++i) p[4 + i * 3] = 128;
  } else {
    s.age[p[1]] = 0;
    for (unsigned i = 0; i < count; ++i) {
      const auto note = p[4 + i * 3];
      if (note < 128) s.age[note] = 0;
      else if (note == 128) {
        // Original age[128] aliases the first two velocity bytes.
        s.velocity[0] = 0; s.velocity[1] = 0;
      }
    }
    s.selected = choose_note(s);
  }
  if (s.selected == old) return false;
  if (old < 128) {
    if (s.selected < 128) { p[0] = 2; p[1] = s.selected; p[2] = s.velocity[s.selected]; }
    else { p[0] = 1; p[1] = old; p[2] = s.velocity[old]; s.source = 0; }
  }
  return true;
}
bool Engine::chord_process(ChordState& s, NotePacket& packet) noexcept {
  auto& p = packet.bytes;
  if (s.mode == 2) {
    if ((p[0] == 0 || p[0] == 2) && (p[3] & 1) && signed_byte(s.learned) < 8 && p[1] < 128) {
      if (s.root >= 128) s.root = p[1];
      else { s.intervals[s.learned] = byte(int(p[1]) - s.root); s.learned = byte(s.learned + 1); }
    }
  } else if (s.mode == 1) {
    const unsigned original = p[0x35];
    for (int j = 0; j < int(s.count) - 1 && p[0x35] < 16; ++j) {
      const auto pitch = byte(s.intervals[static_cast<unsigned>(j)] + p[1]);
      if (pitch < 128) append(packet, pitch, p[2], byte(p[3] | 4));
    }
    for (unsigned i = 0; i < original; ++i) {
      for (int j = 0; j < int(s.count) - 1 && p[0x35] < 16; ++j) {
        const auto pitch = byte(p[4 + i * 3] + s.intervals[static_cast<unsigned>(j)]);
        // Original32e66f indexes auxiliary velocity/flags by interval j.
        if (pitch < 128) append(packet, pitch, p[5 + static_cast<unsigned>(j) * 3],
                               byte(p[6 + static_cast<unsigned>(j) * 3] | 4));
      }
    }
  }
  return true;
}
bool Engine::unison_process(UnisonState& s, NotePacket& packet) noexcept {
  auto& p = packet.bytes;
  if (!s.enabled || s.mode > 1) return true;
  int count = s.count;
  if (s.mode == 0) p[3] = byte(p[3] | 8);
  else {
    const int held = signed_byte(s.held), denominator = held + 1;
    if (count * denominator > 6) count = (held + 6) / denominator;
  }
  const unsigned original = p[0x35];
  for (int repeat = 1; repeat < count && p[0x35] < 16; ++repeat) {
    // Valid primary MIDI pitches are the recovered caller contract.
    append(packet, p[1], p[2], byte(p[3] | 8));
    for (unsigned j = 0; j < original && p[0x35] < 16; ++j) {
      if (p[4 + static_cast<unsigned>(repeat - 1) * 3] < 128) {
        append(packet, p[4 + j * 3], p[5 + j * 3], byte(p[6 + j * 3] | 8));
        p[6 + j * 3] = byte(p[6 + j * 3] | 8);
      }
    }
  }
  if (s.mode == 1) {
    if (p[0] == 0 || p[0] == 2) s.held = byte(s.held + 1);
    else if (p[0] == 1) s.held = byte(s.held - 1);
  }
  return true;
}
void Engine::build_custom_map(ScaleState& s) noexcept {
  std::array<unsigned, 12> lower{}, upper{};
  unsigned first = 12, last = 12;
  for (unsigned i = 0; i < 12; ++i) if (s.mask[i]) { if (first == 12) first = i; last = i; }
  if (first < 12) { lower[0] = first; upper[11] = first + 12; upper[last] = last; }
  for (unsigned i = 1; i < 12; ++i) lower[i] = s.mask[i] ? i : lower[i - 1];
  for (int i = 10; i >= 0; --i) {
    const auto j = static_cast<unsigned>(i); upper[j] = s.mask[j] ? j : upper[j + 1];
  }
  for (unsigned i = 0; i < 12; ++i) {
    const int below = int(i) - int(lower[i]), above = int(upper[i]) - int(i);
    s.map[i] = byte(static_cast<int>(lower[i] == i ? i : below <= above ? lower[i] : upper[i]));
  }
}
bool Engine::scale_process(ScaleState& s, NotePacket& packet) noexcept {
  auto& p = packet.bytes;
  if (s.mode == 2) {
    if ((p[0] == 0 || p[0] == 2) && (p[3] & 1)) {
      auto& enabled = s.mask[p[1] % 12]; enabled = byte(enabled == 0); build_custom_map(s);
    }
    return false;
  }
  if (s.mode != 1 || !s.type || p[1] >= 128) return true;
  const int root = s.type == 7 ? 0 : signed_byte(s.root);
  const auto quantize = [&](std::uint8_t note) {
    const int relative = int(note) - root, octave = relative / 12, remainder = relative - octave * 12;
    const unsigned mapped = s.type == 7 ? s.map[static_cast<unsigned>(remainder)] :
      detail::scale_bytes[static_cast<unsigned>(12 + int(s.type) * 12 + remainder)];
    return byte(octave * 12 + int(mapped) + root);
  };
  p[1] = quantize(p[1]);
  for (unsigned i = 0; i < p[0x35]; ++i) {
    if (p[6 + i * 3] & 8) p[4 + i * 3] = p[1];
    else if (p[4 + i * 3] < 128) {
      const auto mapped = quantize(p[4 + i * 3]);
      bool duplicate = mapped == p[1];
      for (unsigned j = 0; j < i; ++j) duplicate = duplicate || mapped == p[4 + j * 3];
      if (duplicate) p[6 + i * 3] = 0;
      else p[4 + i * 3] = mapped;
    }
  }
  return true;
}

std::uint8_t Engine::free_slot(AllocatorState& s, std::uint8_t pitch, std::uint8_t mode) noexcept {
  const auto free = [&](unsigned i) { return s.rows[i].bytes[0] == 0 && s.rows[i].bytes[7] == 0; };
  if (mode == 2) {
    for (unsigned i = 0; i < s.capacity; ++i) if (free(i)) return byte(static_cast<int>(i));
    return 255;
  }
  if (mode == 1) {
    for (unsigned i = 0; i < s.capacity; ++i) {
      if (s.rows[i].bytes[1] == pitch && free(i)) return byte(static_cast<int>(i));
    }
    return free_slot(s, pitch, 0);
  }
  if (mode != 0) return 255;
  std::uint8_t fallback = 255;
  for (unsigned i = 0; i < s.capacity; ++i) {
    s.cursor = byte(s.cursor + 1);
    if (signed_byte(s.cursor) >= s.capacity) s.cursor = 0;
    const unsigned current = s.cursor;
    if (!free(current)) continue;
    const unsigned other = current % 6 + (current < 6 ? 6U : 0U);
    if (free(other)) return s.cursor;
    if (fallback == 255) fallback = s.cursor;
  }
  if (fallback != 255) return fallback;
  s.cursor = byte(s.cursor + 1);
  return 255;
}
std::uint8_t Engine::steal_slot(const AllocatorState& s, std::uint8_t incoming) noexcept {
  if (s.steal == 2) {
    std::uint32_t least = 0x7fffffff; std::uint8_t selected = 255;
    for (unsigned i = 0; i < s.capacity; ++i) {
      const auto& r = s.rows[i]; const auto age = get32(r, 12);
      if (!r.bytes[0] && age < least) { least = age; selected = byte(static_cast<int>(i)); }
    }
    return selected;
  }
  if (s.steal > 1) return 255;
  unsigned protected_row = 255, low = 127;
  if (s.policy != 4) {
    for (unsigned i = 0; i < s.capacity; ++i) {
      const auto& r = s.rows[i];
      if (r.bytes[0] && (r.bytes[2] & 0xef) == 1 && r.bytes[1] < low) {
        low = r.bytes[1]; protected_row = i;
      }
    }
  }
  std::array<std::int32_t, 5> weights; weights.fill(0x7fffffff);
  std::array<std::uint8_t, 5> selected; selected.fill(255);
  for (unsigned i = 0; i < s.capacity; ++i) {
    if (i == protected_row) continue;
    const auto& r = s.rows[i];
    const auto weight = s.steal == 0 ? std::bit_cast<std::int32_t>(get32(r, 12)) : std::int32_t(r.bytes[3]);
    const auto flags = r.bytes[2]; unsigned category = 5;
    if (!r.bytes[0] && weight < weights[0]) category = 0;
    else if ((incoming == 1 && flags == 1) || ((incoming & 1) && flags == 0x11) ||
             ((incoming & 5) == 5 && (flags & 1))) category = 4;
    else if ((incoming & 1) && (flags & 8)) category = 3;
    else if (flags == 2) category = 2;
    else if ((flags & 12) && (flags & 2)) category = 1;
    if (category < 5 && weight < weights[category]) {
      weights[category] = weight; selected[category] = byte(static_cast<int>(i));
    }
  }
  for (auto i : selected) if (i != 255) return i;
  return 255;
}

bool Engine::allocator_event(AllocatorState& s, AllocatorEvent e, RowCallback callback, void* context) noexcept {
  if (!e.flags || e.pitch > 127) return false;
  if (s.policy > 4) return true;
  if (e.type != 0 && e.type != 2) {
    if (e.type != 1) return true;
    if (s.policy != 0) {
      for (unsigned i = 0; i < s.capacity; ++i) {
        auto& r = s.rows[i];
        if (r.bytes[1] != e.pitch || r.bytes[0] != 1 || !(r.bytes[2] & e.flags)) continue;
        advance_age(s, i);
        if (e.flags & 16) r.bytes[0] = 2;
        else {
          r.bytes[0] = 0; r.bytes[7] = 2;
          s.active_bits = word(unsigned(s.active_bits) & ~(1U << i));
          callback(context, byte(static_cast<int>(i)), r);
        }
      }
    } else {
      auto& r = s.rows[s.selected];
      if (!(e.flags & 1)) {
        if (r.bytes[2] & 1) return false;
        r.bytes[7] = 2;
      }
      if (e.flags & 16) { r.bytes[0] = 2; return true; }
      r.bytes[7] = 2; r.bytes[0] = 0;
      s.active_bits = word(unsigned(s.active_bits) & ~(1U << s.selected));
      callback(context, s.selected, r);
    }
    return true;
  }
  std::uint8_t index = s.selected, status = 1;
  if (s.policy != 0) {
    if (s.policy == 3) {
      index = s.sequence_index; s.sequence_index = byte(s.sequence_index + 1);
      if (index == 255) return false;
      const auto& previous = s.rows[s.selected]; const auto selected_status = s.rows[index].bytes[7];
      if ((!previous.bytes[0] && previous.bytes[7] == 2) || (previous.bytes[0] && previous.bytes[7] != 1)) {
        if (selected_status == 2) status = 4;
        else if (e.type == 2) status = s.legato ? 3 : 4;
      }
    } else {
      index = free_slot(s, e.pitch, s.allocation);
      if (!s.duplicates) {
        for (unsigned i = 0; i < s.capacity; ++i) {
          const auto& r = s.rows[i];
          if ((r.bytes[0] || r.bytes[7] == 2) && r.bytes[1] == e.pitch && !(r.bytes[2] & 8)) {
            if ((r.bytes[2] & 1) && (e.flags & 2)) return false;
            index = byte(static_cast<int>(i)); status = 4; break;
          }
        }
      }
      if (index == 255) {
        index = steal_slot(s, e.flags);
        if (index == 255) return false;
        status = 4;
        if (s.duplicates) {
          const auto pitch = s.rows[index].bytes[1];
          for (unsigned i = 0; i < s.capacity; ++i) if (s.rows[i].bytes[1] == pitch) advance_age(s, i);
        }
      }
    }
  } else {
    const auto& r = s.rows[index]; const auto old = r.bytes[7];
    if (e.flags & 1) status = ((!r.bytes[0] && old != 2) || (r.bytes[0] && old == 1)) ? 1 : s.legato ? 3 : 4;
    else if (e.flags & 2) {
      if (!r.bytes[0] && old != 2) status = 1;
      else { if (r.bytes[2] & 1) return false; status = old == 2 ? 4 : s.legato ? 3 : 4; }
    } else status = old;
  }
  auto& row = s.rows[index]; const auto old_key = row.bytes[18];
  const std::uint8_t carry = s.policy && e.type == 2 && (row.bytes[2] & 16) ? 16 : 0;
  row.bytes[0] = 1; row.bytes[1] = e.pitch; row.bytes[2] = byte(e.flags | carry);
  row.bytes[3] = e.velocity; row.bytes[7] = status; advance_age(s, index);
  row.bytes[8] = e.auxiliary; put16(row, 16, e.word); row.bytes[18] = e.key;
  if (e.key < 128 && s.pitch_table[e.key]) put16(row, 4, s.pitch_table[e.key]);
  else if (old_key < 128 && s.pitch_table[old_key]) put16(row, 4, 0);
  s.active_bits = word(unsigned(s.active_bits) | (1U << index));
  callback(context, index, row);
  return true;
}

void Engine::publish(std::uint8_t row, const NoteRow& value) noexcept {
  const auto status = value.bytes[7];
  if (status == 1 || status == 4) state_.on_count = byte(state_.on_count + 1);
  if (status == 2 || status == 4 || status == 5) state_.off_count = byte(state_.off_count + 1);
  published_[row] = value; published_valid_ = word(unsigned(published_valid_) | (1U << row));
  if (events_.count < events_.entries.size()) events_.entries[events_.count++] = {row, value};
  if (value.bytes[0] == 1) state_.last_active_row = row;
}
void Engine::on_row(void* context, std::uint8_t row, const NoteRow& value) noexcept {
  static_cast<Engine*>(context)->publish(row, value);
}
void Engine::clear_row(std::uint8_t row, std::uint8_t status) noexcept {
  auto& r = state_.allocator.rows[row];
  r.bytes[0] = 0; put16(r, 4, 0); put32(r, 12, 0); r.bytes[7] = status;
  r.bytes[8] = 0; put16(r, 16, 0);
  publish(row, r);
  r.bytes[2] = 0;
}
void Engine::clear_statuses() noexcept {
  for (auto& r : state_.allocator.rows) r.bytes[7] = 0;
}
void Engine::prepare_mode(ModeControls controls) noexcept {
  begin_operation();
  const int note = discrete(controls.note_mode_q15, 4);
  const int allocation = discrete(controls.allocation_q15, 3);
  const int unison = discrete(controls.unison_mode_q15, 3);
  int policy = 1, capacity = 6;
  bool mono = false, unison_enabled = false, split = false;
  if (note == 0) { policy = 0; mono = true; }
  else if (note == 1) {
    policy = 4; unison_enabled = true;
    if (unison == 0) { policy = 3; mono = true; }
    else if (unison == 1) { policy = 1; split = true; }
    else if (unison == 2) capacity = 12;
    else capacity = 0;
  } else if (note == 2) policy = 1;
  else if (note == 3) { policy = 2; capacity = 12; }
  else return;
  if (controls.external_mode_override) policy = mono ? 3 : 4;
  reset_poly(state_.poly);
  for (unsigned i = 0; i < voice_rows; ++i) clear_row(byte(static_cast<int>(i)), 5);
  state_.allocator.pitch_table.fill(0);
  auto& a = state_.allocator; a.policy = byte(policy);
  if (policy == 0) a.capacity = 1;
  else if (policy == 1) a.capacity = 6;
  else if (policy == 2) a.capacity = 12;
  a.cursor = byte(signed_byte(a.cursor) % int(a.capacity));
  a.allocation = byte(allocation); a.duplicates = byte(split);
  state_.poly.enabled = byte(mono); state_.unison.enabled = byte(unison_enabled);
  state_.unison.mode = byte(split); a.unison_table = byte(unison_enabled);
  if (unison_enabled || controls.external_mode_override) a.capacity = byte(capacity);
}
void Engine::prepare_unison_count(std::int16_t q15) noexcept {
  begin_operation();
  const int n = (int(q15) * 5 + 2 + (q15 < 0 ? -4 : 0)) / 32768 + 2;
  state_.unison.count = byte(n); state_.allocator.parity = byte(n & 1);
}
void Engine::prepare_legato(std::int16_t q15) noexcept {
  begin_operation(); state_.allocator.legato = byte(q15 != 0); state_.poly.legato = byte(q15 != 0);
}
void Engine::configure_chord(ChordState s) noexcept { begin_operation(); state_.chord = s; }
void Engine::configure_scale(ScaleState s) noexcept { begin_operation(); state_.scale = s; }
void Engine::allocator_batch(NotePacket& packet) noexcept {
  auto& a = state_.allocator; const auto& p = packet.bytes;
  const auto& auxiliary = !a.unison_table ? detail::auxiliary0 : a.parity ? detail::auxiliary1 : detail::auxiliary2;
  a.sequence_index = 0;
  const auto event = [&](std::uint8_t pitch, std::uint8_t velocity, std::uint8_t flags,
                         std::uint8_t aux, std::uint16_t value) {
    return allocator_event(a, {p[0], pitch, velocity, flags, aux, value, p[0x38]}, on_row, this);
  };
  const auto extra_word = word(unsigned(p[0x36]) | (unsigned(p[0x37]) << 8));
  event(p[1], p[2], p[3], auxiliary[0], extra_word);
  const int count = signed_byte(p[0x35]);
  if (p[0] == 0 || p[0] == 2) {
    for (int i = 0; i < count && i < int(a.capacity) - 1; ++i) {
      const auto j = static_cast<unsigned>(i);
      event(p[4 + j * 3], p[5 + j * 3], p[6 + j * 3], auxiliary[j + 1], extra_word);
    }
  } else if (p[0] == 1) {
    for (int i = 0; i < count; ++i) {
      const auto j = static_cast<unsigned>(i);
      event(p[4 + j * 3], p[5 + j * 3], p[6 + j * 3], auxiliary[j + 1], 0);
    }
  }
  if (a.policy == 3 && p[0] == 2) {
    for (unsigned i = a.sequence_index; i < a.capacity; ++i) {
      if (a.rows[i].bytes[0]) allocator_event(a, {1, a.rows[i].bytes[1], 64, p[3], 0, 0, p[0x38]}, on_row, this);
    }
  }
}
void Engine::pipeline(NotePacket& p) noexcept {
  if (poly_process(state_.poly, p) && chord_process(state_.chord, p) &&
      unison_process(state_.unison, p) && scale_process(state_.scale, p)) allocator_batch(p);
}
void Engine::feed_midi_note(bool on, std::uint8_t pitch, std::uint8_t velocity) noexcept {
  begin_operation();
  if (state_.status_pending) clear_statuses();
  state_.last_input = empty_packet(); auto& p = state_.last_input.bytes;
  p[0] = byte(!on); p[1] = pitch; p[2] = velocity; p[3] = state_.held ? 17 : 1; p[0x38] = pitch;
  pipeline(state_.last_input); state_.status_pending = 0;
}
void Engine::feed(NotePacket& p) noexcept {
  begin_operation();
  if (state_.status_pending && !state_.suppress_status_clear) {
    clear_statuses(); state_.status_pending = 0;
  }
  pipeline(p);
}
HoldDispatch Engine::update_hold(std::int16_t q15, bool dirty, bool force) noexcept {
  begin_operation();
  if (!dirty && !force) return {false, 0, 0};
  const bool held = q15 >= 16384;
  if (state_.held && !held) {
    for (unsigned i = 0; i < voice_rows; ++i) {
      const auto& r = state_.allocator.rows[i];
      if (r.bytes[0] == 2 || (r.bytes[2] & 2)) clear_row(byte(static_cast<int>(i)), 2);
    }
    state_.allocator.pitch_table.fill(0);
  }
  state_.held = byte(held);
  if (held) {
    for (unsigned i = 0; i < state_.allocator.capacity; ++i) {
      auto& r = state_.allocator.rows[i]; if (r.bytes[0] == 1) r.bytes[2] = byte(r.bytes[2] | 16);
    }
  }
  return {true, 32, q15};
}
HoldDispatch Engine::update_hold_normalized(float normalized, bool dirty, bool force) noexcept {
  if (!dirty && !force) { begin_operation(); return {false, 0, 0}; }
  const auto q15 = static_cast<std::int32_t>(normalized * 32767.0f);
  return update_hold(std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(q15)), dirty, force);
}
void Engine::all_notes_off(bool requested) noexcept {
  begin_operation(); if (!requested) return;
  const auto held = state_.held; state_.held = 0; reset_poly(state_.poly);
  for (unsigned i = 0; i < voice_rows; ++i) clear_row(byte(static_cast<int>(i)), 5);
  state_.allocator.pitch_table.fill(0); state_.held = held;
}
void Engine::finish_control_block() noexcept {
  begin_operation(); state_.status_pending = 1; state_.on_count = 0; state_.off_count = 0;
}

}
