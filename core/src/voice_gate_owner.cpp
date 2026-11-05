#include "mfrk/voice_gate_owner.hpp"
namespace mfrk::voice_gate {
namespace {
bool ready(Owner& owner,const gate_dispatch::Action* actions,unsigned count) noexcept {
 for(unsigned i=0;i<count;++i){const auto& a=actions[i];const auto index=unsigned(a.voice)*2+a.side;
  if(a.kind==gate_dispatch::Kind::oscillator&&!owner.slots[index])return false;
  if(a.kind==gate_dispatch::Kind::main&&(!owner.main_envelopes[index]||!owner.main_envelopes[index]->prepared()))return false;
  if(a.kind==gate_dispatch::Kind::cyclic&&!owner.cyclic)return false;
 }
 return true;
}
void apply(Owner& owner,const gate_dispatch::Action* actions,unsigned count) noexcept {
 for(unsigned i=0;i<count;++i){const auto& a=actions[i];const auto index=unsigned(a.voice)*2+a.side;
  if(a.kind==gate_dispatch::Kind::oscillator)owner.slots[index]->trigger(a.on?1:0);
  else if(a.kind==gate_dispatch::Kind::main)owner.main_envelopes[index]->gate(a.on);
  else owner.cyclic(owner.cyclic_context,a.voice,a.side,a.on);
 }
}
}
bool dispatch(Owner& owner,gate_dispatch::State& state,std::uint8_t row,std::uint8_t status,bool para,std::uint8_t cyclic_mode,bool disabled) noexcept {
 auto next=state;gate_dispatch::Plan plan;
 if(!gate_dispatch::dispatch(next,row,status,para,cyclic_mode,disabled,plan)||!ready(owner,plan.actions.data(),plan.count))return false;
 apply(owner,plan.actions.data(),plan.count);state=next;return true;
}
bool process_rows(Owner& owner,gate_dispatch::State& state,const std::array<notepath::NoteRow,12>& rows,bool para,std::uint8_t cyclic_mode,bool disabled) noexcept {
 auto next=state;gate_dispatch::RowsPlan plan;
 if(!gate_dispatch::plan_rows(next,rows,para,cyclic_mode,disabled,plan)||!ready(owner,plan.actions.data(),plan.count))return false;
 apply(owner,plan.actions.data(),plan.count);state=next;return true;
}
}
