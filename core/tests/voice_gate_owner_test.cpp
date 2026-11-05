#include "mfrk/voice_gate_owner.hpp"
#include "mfrk/publication_sink.hpp"
#include "mfrk/phase_reset.hpp"
#include "adsr_test_access.hpp"
#include <memory>
using namespace mfrk;
struct CyclicLog {unsigned calls=0;bool last=false;};
static void cyclic(void* p,std::uint8_t voice,std::uint8_t side,bool on) noexcept {auto& log=*static_cast<CyclicLog*>(p);if(voice==0&&side==0){++log.calls;log.last=on;}}
static void reset(void* p,gate_dispatch::Kind,std::uint8_t,std::uint8_t) noexcept {++*static_cast<unsigned*>(p);}
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
 std::array<std::unique_ptr<core_bank::Side>,12> sides;std::array<std::unique_ptr<core_bank::SlotController>,12> slots;std::array<std::unique_ptr<Adsr>,12> envs;voice_gate::Owner startup;unsigned resets=0;startup.reset=reset;startup.reset_context=&resets;
 for(unsigned i=0;i<12;++i){sides[i]=std::make_unique<core_bank::Side>(*tables,random,i+10);slots[i]=std::make_unique<core_bank::SlotController>(*sides[i]);envs[i]=std::make_unique<Adsr>();initial={};initial.phase=AdsrPhase::sustain;initial.accumulator=0x40000000;AdsrTestAccess::load_explicit_fixture(*envs[i],initial);startup.slots[i]=slots[i].get();startup.main_envelopes[i]=envs[i].get();}
 std::array<notepath::NoteRow,12> reset_rows{};for(auto& row:reset_rows)row.bytes[7]=5;gate_dispatch::State startup_state;
 if(!voice_gate::process_rows(startup,startup_state,reset_rows,false,0,false)||resets!=36)return 9;
 for(const auto& e:envs)if(e->snapshot().phase!=AdsrPhase::idle)return 10;
 Adsr cyc_env;initial={};initial.phase=AdsrPhase::idle;AdsrTestAccess::load_explicit_fixture(cyc_env,initial);AdsrTestAccess::load_explicit_fixture(env,initial);
 phase_reset::Cyclic cyc;cyc.envelope=&cyc_env;cyc.phase={.2f,.5f,.75f,7.f,8.f};cyc.flag93=127;phase_reset::Phase lfo1{.2f,.5f,.75f,7.f,8.f},lfo2=lfo1;phase_reset::Bank modulators;modulators.cyclic[0]=&cyc;modulators.lfo1[0]=&lfo1;modulators.lfo2[0]=&lfo2;
 owner.cyclic=phase_reset::Bank::gate_callback;owner.cyclic_ready=phase_reset::Bank::can_gate;owner.cyclic_context=&modulators;owner.reset=phase_reset::Bank::reset_callback;owner.reset_ready=phase_reset::Bank::can_reset;owner.reset_context=&modulators;state={};
 if(!voice_gate::dispatch(owner,state,0,1,false,0,false)||cyc_env.snapshot().phase!=AdsrPhase::attack||cyc.flag92!=1||cyc.flag95!=127)return 11;
 if(!voice_gate::dispatch(owner,state,0,5,false,0,false)||cyc_env.snapshot().phase!=AdsrPhase::idle||lfo1.current!=cyc.phase.current||lfo2.current!=cyc.phase.current)return 12;
 gate_dispatch::ResetContext other{false,false};const auto before_count=state.active_count;
 if(voice_gate::dispatch(owner,state,0,5,true,0,false,&other)||state.active_count!=before_count)return 13;
 phase_reset::Shared shared{9,8};modulators.shared[0]=&shared;
 if(!voice_gate::dispatch(owner,state,0,5,true,0,false,&other)||shared.field18!=0||shared.field60!=3)return 14;
}
