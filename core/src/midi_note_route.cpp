#include "mfrk/midi_note_route.hpp"
namespace mfrk::midi {
NoteResult route_note(const Event& event,unsigned channel,notepath::Engine& actor,PublicationSink sink,void* context) noexcept {
 if(channel>16||!sink)return NoteResult::invalid;
 if(event.sysex_byte||((event.status&0xf0)!=0x80&&(event.status&0xf0)!=0x90))return NoteResult::unsupported;
 if(event.size!=2||event.data[0]>127||event.data[1]>127)return NoteResult::invalid;
 if(channel!=16&&(event.status&0x0f)!=channel)return NoteResult::filtered;
 const bool on=(event.status&0xf0)==0x90&&event.data[1]!=0;
 actor.feed_midi_note(on,event.data[0],event.data[1]);
 sink(context,actor.publications());
 return NoteResult::delivered;
}
}
