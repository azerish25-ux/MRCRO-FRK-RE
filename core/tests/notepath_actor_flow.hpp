#pragma once
#include "mfrk/notepath.hpp"

namespace mfrk::notepath::example {
// Call immediately after EVERY public actor operation. Sink.apply must perform
// the actual owner handoff once. It must not call another actor operation.
template<class RowSink>
void apply_current_batch(const Engine& actor,RowSink& sink) noexcept {
  const auto& batch=actor.publications();
  for(unsigned i=0;i<batch.count;++i){const auto& event=batch.entries[i];sink.apply(event.row,event.value);}
}
template<class RowSink>
void receive_note(Engine& actor,RowSink& sink,bool on,std::uint8_t pitch,std::uint8_t velocity) noexcept {
  actor.feed_midi_note(on,pitch,velocity);
  apply_current_batch(actor,sink);
}
template<class RowSink>
void receive_canonical(Engine& actor,RowSink& sink,NotePacket& packet) noexcept {
  actor.feed(packet);
  apply_current_batch(actor,sink);
}
template<class RowSink>
HoldDispatch receive_hold(Engine& actor,RowSink& sink,float normalized,bool dirty,bool force) noexcept {
  const auto dispatch=actor.update_hold_normalized(normalized,dirty,force);
  apply_current_batch(actor,sink);
  // The owner also forwards processed dispatch{id32,value} to its sequencer.
  return dispatch;
}
template<class RunVoiceBlock,class ConsumeCounts>
void finish_voice_block(Engine& actor,RunVoiceBlock&& run_voices,ConsumeCounts&& consume_counts) noexcept {
  // All earlier operation batches have ALREADY been applied exactly once.
  // The concrete owner processes its voice/control rows and consumes statuses.
  run_voices();
  consume_counts(actor.note_counts());
  actor.finish_control_block();
  // published_rows() retains snapshots; do not replay them on the next block.
}
} // namespace mfrk::notepath::example
