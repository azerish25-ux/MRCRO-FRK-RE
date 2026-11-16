#pragma once
#include "mfrk/preset_program.hpp"
#include <array>
namespace mfrk::wheels_listener {
struct State {std::array<std::uint32_t,4> ids;bool busy;};
// Source 5e1b90 scalar callback. Distinct stable pair bindings, no exceptions
// or reentry. suppress() supplies the native IParam+128 policy; write() receives
// (original origin, paired targetID, raw bits, busy flag). Receiver coercion,
// constructor, suppression-policy derivation and controller delivery are separate.
template<class Boundary>
void notify(State& state,std::uint32_t event,std::uint32_t bits,std::uint32_t origin,Boundary& boundary){
 if(origin==3||origin==0)return;
 if(boundary.suppress())return;
 if(state.busy)return;
 state.busy=true;
 for(std::size_t i=0;i<4;++i)if(event==state.ids[i]){boundary.write(origin,state.ids[i^1u],bits,state.busy);break;}
 state.busy=false;
}
template<class Boundary>
bool notify(State& state,const preset_program::Command& command,Boundary& boundary){
 if(command.tag!=1||command.payload.size()!=4)return false;
 std::uint32_t bits=0;for(unsigned i=0;i<4;++i)bits|=std::uint32_t(command.payload[i])<<(8*i);
 notify(state,command.wrapper_id,bits,command.origin,boundary);return true;
}
}
