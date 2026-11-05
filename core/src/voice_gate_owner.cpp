#include "mfrk/voice_gate_owner.hpp"
namespace mfrk::voice_gate {
namespace {
bool ready(Owner& owner,const gate_dispatch::Action* actions,unsigned count) noexcept {
 for(unsigned i=0;i<count;++i){const auto& a=actions[i];const auto index=unsigned(a.voice)*2+a.side;
  if(a.kind==gate_dispatch::Kind::oscillator&&!owner.slots[index])return false;
  if(a.kind==gate_dispatch::Kind::main&&(!owner.main_envelopes[index]||!owner.main_envelopes[index]->prepared()))return false;
  if(a.kind==gate_dispatch::Kind::main_reset&&!owner.main_envelopes[index])return false;
  if(a.kind==gate_dispatch::Kind::cyclic&&(!owner.cyclic||(owner.cyclic_ready&&!owner.cyclic_ready(owner.cyclic_context,a.voice,a.side,a.on))))return false;
  if(a.kind==gate_dispatch::Kind::cyclic_reset||a.kind==gate_dispatch::Kind::lfo1_reset||a.kind==gate_dispatch::Kind::lfo2_reset||a.kind==gate_dispatch::Kind::voice_reset){if(!owner.reset||(owner.reset_ready&&!owner.reset_ready(owner.reset_context,a.kind,a.voice,a.side)))return false;}
 }
 return true;
}
void apply(Owner& owner,const gate_dispatch::Action* actions,unsigned count) noexcept {
 for(unsigned i=0;i<count;++i){const auto& a=actions[i];const auto index=unsigned(a.voice)*2+a.side;
  if(a.kind==gate_dispatch::Kind::oscillator)owner.slots[index]->trigger(a.on?1:0);
  else if(a.kind==gate_dispatch::Kind::main)owner.main_envelopes[index]->gate(a.on);
  else if(a.kind==gate_dispatch::Kind::cyclic)owner.cyclic(owner.cyclic_context,a.voice,a.side,a.on);
  else if(a.kind==gate_dispatch::Kind::main_reset)owner.main_envelopes[index]->reset();
  else owner.reset(owner.reset_context,a.kind,a.voice,a.side);
 }
}
}
bool dispatch(Owner& owner,gate_dispatch::State& state,std::uint8_t row,std::uint8_t status,bool para,std::uint8_t cyclic_mode,bool disabled,const gate_dispatch::ResetContext* reset_context) noexcept {
 auto next=state;gate_dispatch::Plan plan;
 if(!gate_dispatch::dispatch(next,row,status,para,cyclic_mode,disabled,plan,reset_context)||!ready(owner,plan.actions.data(),plan.count))return false;
 apply(owner,plan.actions.data(),plan.count);state=next;return true;
}
bool process_rows(Owner& owner,gate_dispatch::State& state,const std::array<notepath::NoteRow,12>& rows,bool para,std::uint8_t cyclic_mode,bool disabled,const std::uint16_t* row_mask) noexcept {
 auto next=state;gate_dispatch::RowsPlan plan;
 if(!gate_dispatch::plan_rows(next,rows,para,cyclic_mode,disabled,plan,row_mask)||!ready(owner,plan.actions.data(),plan.count))return false;
 apply(owner,plan.actions.data(),plan.count);state=next;return true;
}
}
