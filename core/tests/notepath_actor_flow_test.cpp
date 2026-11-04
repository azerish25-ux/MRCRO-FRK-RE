#include "notepath_actor_flow.hpp"
#include <array>
#include <cstdint>
using namespace mfrk::notepath;
struct Sink {
 unsigned calls=0;std::array<NoteRow,voice_rows> rows{};
 void apply(std::uint8_t row,const NoteRow& value) noexcept {++calls;rows[row]=value;}
};
int main(){auto actor=Engine::admitted_zero_arena();Sink sink;
 actor.prepare_mode({16384,0,0,0});example::apply_current_batch(actor,sink);
 if(sink.calls!=12)return 1;
 actor.prepare_unison_count(0);example::apply_current_batch(actor,sink);
 actor.prepare_legato(32767);example::apply_current_batch(actor,sink);
 if(sink.calls!=12)return 2; // Empty new batches do not replay prior kills.
 example::receive_note(actor,sink,true,60,100);if(sink.calls!=13)return 3;
 // Mode kills have status5 until the next consumed block; first note is
 // original status4, so offcount includes12kills+1retrigger.
 bool ran=false,consumed=false;
 example::finish_voice_block(actor,[&]() noexcept {ran=true;},[&](NoteCounts counts) noexcept {consumed=ran&&counts.on==1&&counts.off==13;});
 if(!consumed||actor.note_counts().on||actor.note_counts().off||actor.publications().count)return 4;
 example::receive_note(actor,sink,false,60,64);if(sink.calls!=14||actor.note_counts().off!=1)return 5;
 if(actor.published_valid_bits()!=0xfff)return 6;
 return 0;
}
