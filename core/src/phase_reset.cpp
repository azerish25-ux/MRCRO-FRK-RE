#include "mfrk/phase_reset.hpp"
namespace mfrk::phase_reset {
void reset(Phase& state) noexcept {
 const float product=state.factor_a*state.factor_b;
 state.previous=state.reference;
 float current=state.reference-product;
 if(current<0.f)current=current+1.f;
 state.current=current;
}
void reset_cyclic(Adsr& envelope,Phase& phase) noexcept {envelope.reset();reset(phase);}
void reset_shared(Shared& state) noexcept {state.field18=0;state.field60=3;}
bool gate(Cyclic& state,bool on) noexcept {
 if(!state.envelope||!state.envelope->gate(on))return false;
 if(on){state.flag95=state.flag93;state.flag91=0;state.flag92=1;reset(state.phase);}
 return true;
}
bool Bank::can_gate(void* opaque,std::uint8_t voice,std::uint8_t side,bool) noexcept {
 if(!opaque||voice>=6||side>=2)return false;const auto* value=static_cast<Bank*>(opaque)->cyclic[voice*2+side];return value&&value->envelope&&value->envelope->prepared();
}
void Bank::gate_callback(void* opaque,std::uint8_t voice,std::uint8_t side,bool on) noexcept {gate(*static_cast<Bank*>(opaque)->cyclic[voice*2+side],on);}
bool Bank::can_reset(void* opaque,gate_dispatch::Kind kind,std::uint8_t voice,std::uint8_t side) noexcept {
 if(!opaque||voice>=6||side>=2)return false;const auto& bank=*static_cast<Bank*>(opaque);const unsigned i=voice*2+side;
 if(kind==gate_dispatch::Kind::voice_reset)return bank.shared[voice]!=nullptr;
 if(kind==gate_dispatch::Kind::cyclic_reset)return bank.cyclic[i]&&bank.cyclic[i]->envelope;
 if(kind==gate_dispatch::Kind::lfo1_reset)return bank.lfo1[i]!=nullptr;
 if(kind==gate_dispatch::Kind::lfo2_reset)return bank.lfo2[i]!=nullptr;
 return false;
}
void Bank::reset_callback(void* opaque,gate_dispatch::Kind kind,std::uint8_t voice,std::uint8_t side) noexcept {
 auto& bank=*static_cast<Bank*>(opaque);const unsigned i=voice*2+side;
 if(kind==gate_dispatch::Kind::voice_reset)reset_shared(*bank.shared[voice]);
 else if(kind==gate_dispatch::Kind::cyclic_reset)reset_cyclic(*bank.cyclic[i]->envelope,bank.cyclic[i]->phase);
 else if(kind==gate_dispatch::Kind::lfo1_reset)reset(*bank.lfo1[i]);
 else if(kind==gate_dispatch::Kind::lfo2_reset)reset(*bank.lfo2[i]);
}
}
