#include "mfrk/midi_note_route.hpp"
#include "mfrk/publication_sink.hpp"
#include <array>
using namespace mfrk;
struct Sink {notepath::Publications batch{};publication_sink::Owner owner{};unsigned calls=0;bool ok=true;};
static void publish(void* p,const notepath::Publications& batch) noexcept {auto& s=*static_cast<Sink*>(p);s.batch=batch;s.ok&=publication_sink::publish_batch(s.owner,batch,0);++s.calls;}
static bool same(const notepath::Publications& a,const notepath::Publications& b){if(a.count!=b.count)return false;for(unsigned i=0;i<a.count;++i)if(a.entries[i].row!=b.entries[i].row||a.entries[i].value.bytes!=b.entries[i].value.bytes)return false;return true;}
int main(){
 auto routed=notepath::Engine::admitted_zero_arena();auto direct=notepath::Engine::admitted_zero_arena();Sink sink;
 midi::Event on{0x92,{60,100},2,false};
 if(midi::route_note(on,16,routed,publish,&sink)!=midi::NoteResult::delivered)return 1;
 direct.feed_midi_note(true,60,100);if(sink.calls!=1||!sink.ok||sink.batch.count==0||!same(sink.batch,direct.publications()))return 2;
 for(unsigned i=0;i<sink.batch.count;++i)if(sink.owner.rows[sink.batch.entries[i].row].bytes!=sink.batch.entries[i].value.bytes)return 7;
 midi::Event off{0x92,{60,0},2,false};
 if(midi::route_note(off,2,routed,publish,&sink)!=midi::NoteResult::delivered)return 3;
 direct.feed_midi_note(false,60,0);if(sink.calls!=2||!same(sink.batch,direct.publications()))return 4;
 if(midi::route_note(on,1,routed,publish,&sink)!=midi::NoteResult::filtered||sink.calls!=2)return 5;
 midi::Event control{0xb2,{64,127},2,false};if(midi::route_note(control,16,routed,publish,&sink)!=midi::NoteResult::unsupported)return 6;
}
