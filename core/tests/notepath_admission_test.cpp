#include "mfrk/notepath.hpp"
#include <array>
#include <type_traits>
#include <utility>
using namespace mfrk::notepath;
template<class T> concept HasRawStateGetter=requires(const T& actor){actor.state();};
static_assert(!std::is_default_constructible_v<Engine>);
static_assert(!HasRawStateGetter<Engine>);
static_assert(noexcept(Engine::admitted_zero_arena()));
static_assert(noexcept(std::declval<Engine&>().finish_control_block()));
static_assert(sizeof(NotePacket)==58&&sizeof(NoteRow)==20&&sizeof(PerNoteKey)==1);
int main(){
  auto actor=Engine::admitted_zero_arena();
  if(actor.published_valid_bits()!=0||actor.publications().count!=0||actor.note_counts().on||actor.note_counts().off)return 1;
  NotePacket packet;packet.set_per_note_key({255});if(packet.per_note_key().value!=255)return 2;
  std::array<NoteRow,voice_rows> observed{}; // Test sink only, not native MNF defaults.
  const auto apply=[&]() noexcept {
    const auto& batch=actor.publications();
    for(unsigned i=0;i<batch.count;++i)observed[batch.entries[i].row]=batch.entries[i].value;
  };
  ChordState chord;chord.mode=2;chord.root=128;actor.configure_chord(chord);apply();
  actor.feed_midi_note(true,64,100);apply();actor.feed_midi_note(true,67,100);apply();
  if(actor.chord_state().root!=64||actor.chord_state().learned!=1||actor.chord_state().intervals[0]!=3)return 3;
  ScaleState scale;scale.mode=2;scale.type=7;actor.configure_scale(scale);apply();
  actor.feed_midi_note(true,60,100);apply();
  return actor.scale_state().mask[0]==1?0:4;
}
