#include "mfrk/voice_gate_owner.hpp"
namespace mfrk::voice_gate {
bool dispatch(Owner& owner,gate_dispatch::State& state,std::uint8_t row,std::uint8_t status,bool para,std::uint8_t cyclic_mode,bool disabled) noexcept {
 auto next=state;gate_dispatch::Plan plan;
 if(!gate_dispatch::dispatch(next,row,status,para,cyclic_mode,disabled,plan))return false;
 for(unsigned i=0;i<plan.count;++i){const auto& a=plan.actions[i];const auto index=unsigned(a.voice)*2+a.side;
  if(a.kind==gate_dispatch::Kind::oscillator&&!owner.slots[index])return false;
  if(a.kind==gate_dispatch::Kind::main&&(!owner.main_envelopes[index]||!owner.main_envelopes[index]->prepared()))return false;
  if(a.kind==gate_dispatch::Kind::cyclic&&!owner.cyclic)return false;
 }
 for(unsigned i=0;i<plan.count;++i){const auto& a=plan.actions[i];const auto index=unsigned(a.voice)*2+a.side;
  if(a.kind==gate_dispatch::Kind::oscillator)owner.slots[index]->trigger(a.on?1:0);
  else if(a.kind==gate_dispatch::Kind::main)owner.main_envelopes[index]->gate(a.on);
  else owner.cyclic(owner.cyclic_context,a.voice,a.side,a.on);
 }
 state=next;return true;
}
}
