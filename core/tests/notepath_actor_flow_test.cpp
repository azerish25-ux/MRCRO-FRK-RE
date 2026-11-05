#include "notepath_actor_flow.hpp"
#include <array>
#include <cstdint>
using namespace mfrk::notepath;
struct Sink {
 unsigned calls=0;std::array<NoteRow,voice_rows> rows{};
 void apply(std::uint8_t row,const NoteRow& value) noexcept {++calls;rows[row]=value;}
};
int main(){
 auto actor=Engine::admitted_zero_arena();Sink sink;
 actor.prepare_mode({16384,0,0,0});example::apply_current_batch(actor,sink);
 if(sink.calls!=12)return 1;
 actor.prepare_unison_count(0);example::apply_current_batch(actor,sink);
 actor.prepare_legato(32767);example::apply_current_batch(actor,sink);
 if(sink.calls!=12)return 2;
 example::receive_note(actor,sink,true,60,100);if(sink.calls!=13)return 3;
 // Native Mini2f3760 processes the sequencer first. Counts then reach the
 // FX consumer, finish sets pending/clears counts, and only then the remaining
 // owner continuation runs (post-sequencer events and main MNF voice audio).
 bool continued=false,consumed=false,clear_before_continuation=false;
 example::finish_voice_block(actor,[&]() noexcept {
   clear_before_continuation=consumed&&actor.note_counts().on==0&&
       actor.note_counts().off==0&&actor.publications().count==0;
   continued=true;
 },[&](NoteCounts counts) noexcept {
   consumed=!continued&&counts.on==1&&counts.off==13;
 });
 if(!consumed||!continued||!clear_before_continuation)return 4;
 // The original owner skips its count consumer when both wrapping bytes are0.
 bool zero_consumer_called=false,empty_continued=false;
 example::finish_voice_block(actor,[&]() noexcept {
   empty_continued=actor.note_counts().on==0&&actor.note_counts().off==0;
 },[&](NoteCounts) noexcept {zero_consumer_called=true;});
 if(zero_consumer_called||!empty_continued)return 5;
 // A note operation from the next sequencer/control stage is applied before
 // this boundary; its count must be consumed before continuing the owner.
 example::receive_note(actor,sink,false,60,64);
 bool release_consumed=false,release_continued=false;
 example::finish_voice_block(actor,[&]() noexcept {
   release_continued=release_consumed&&actor.note_counts().on==0&&
       actor.note_counts().off==0&&actor.publications().count==0;
 },[&](NoteCounts counts) noexcept {
   release_consumed=counts.on==0&&counts.off==1;
 });
 if(sink.calls!=14||!release_consumed||!release_continued)return 6;
 if(actor.published_valid_bits()!=0xfff)return 7;
 return 0;
}
