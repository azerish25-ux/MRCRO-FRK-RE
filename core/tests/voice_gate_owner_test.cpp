#include "mfrk/voice_gate_owner.hpp"
#include "mfrk/publication_sink.hpp"
#include "adsr_test_access.hpp"
#include <memory>
using namespace mfrk;
struct CyclicLog {unsigned calls=0;bool last=false;};
static void cyclic(void* p,std::uint8_t voice,std::uint8_t side,bool on) noexcept {auto& log=*static_cast<CyclicLog*>(p);if(voice==0&&side==0){++log.calls;log.last=on;}}
int main(){
 auto tables=std::make_unique<core_bank::Tables>();karplus::SharedRandom random{54321};auto first=std::make_unique<core_bank::Side>(*tables,random,1);auto second=std::make_unique<core_bank::Side>(*tables,random,2);core_bank::SlotController slot0(*first),slot1(*second);
 Adsr env;AdsrState initial{};initial.phase=AdsrPhase::sustain;initial.accumulator=0x40000000;initial.hold=1;AdsrTestAccess::load_explicit_fixture(env,initial);
 CyclicLog log;voice_gate::Owner owner;owner.slots[0]=&slot0;owner.slots[1]=&slot1;owner.main_envelopes[0]=&env;owner.cyclic=cyclic;owner.cyclic_context=&log;gate_dispatch::State state;state.active_count=1;
 if(!voice_gate::dispatch(owner,state,0,2,false,0,false))return 1;
 if(state.active_count!=0||env.snapshot().phase!=AdsrPhase::sustain||env.snapshot().pending_release!=1||log.calls!=1||log.last)return 2;
 initial={};initial.phase=AdsrPhase::idle;AdsrTestAccess::load_explicit_fixture(env,initial);
 if(!voice_gate::dispatch(owner,state,0,1,false,0,false))return 3;
 if(state.active_count!=1||env.snapshot().phase!=AdsrPhase::attack||log.calls!=2||!log.last)return 4;
 const auto before=env.snapshot();owner.slots[1]=nullptr;
 if(voice_gate::dispatch(owner,state,0,2,false,0,false)||state.active_count!=1||env.snapshot().phase!=before.phase||log.calls!=2)return 5;
 owner.slots[1]=&slot1;state={};initial={};initial.phase=AdsrPhase::idle;AdsrTestAccess::load_explicit_fixture(env,initial);
 auto notes=notepath::Engine::admitted_zero_arena();publication_sink::Owner published;notes.feed_midi_note(true,60,100);
 if(!notes.publications().count||!publication_sink::publish_batch(published,notes.publications(),0))return 6;
 if(!voice_gate::process_rows(owner,state,published.rows,false,0,false)||state.active_count!=1||env.snapshot().phase!=AdsrPhase::attack||log.calls!=3)return 7;
 publication_sink::finish_voice_block(published);
 if(!voice_gate::process_rows(owner,state,published.rows,false,0,false)||state.active_count!=1||log.calls!=3)return 8;
}
