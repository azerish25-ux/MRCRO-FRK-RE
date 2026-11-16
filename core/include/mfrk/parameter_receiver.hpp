#pragma once
#include "mfrk/preset_bank.hpp"
namespace mfrk::parameter_receiver {
// Source 6d1ae0 for initialized InternalParam receivers (their immediate +50
// hook is the native no-op). Moves incoming; a distinct incoming value becomes
// scalar zero. A self-move retains the value. Only scalar equality can keep a
// previously clean receiver clean; an incoming data value always dirties it.
// Mode 1 always notifies, mode 2 notifies if dirty, other byte values defer.
// Dirty is cleared BEFORE Boundary.notify(id, sharedValue, origin), which may
// synchronously recurse. Returns whether notification was invoked. No positive
// VST immediate hook/cache, FP status, exceptions or concurrent access claim.
template<class Boundary>
bool set(preset_bank::InternalReceiver& receiver,preset_bank::Value&& incoming,
 std::uint32_t origin,std::uint8_t mode,Boundary& boundary){
 const bool equal=incoming.tag()==1&&receiver.value.tag()==1&&parameter_value_bits::equal_scalar(receiver.value.scalar_bits(),incoming.scalar_bits());
 receiver.dirty=receiver.dirty||!equal;
 receiver.value=std::move(incoming);
 if(mode==1||(mode==2&&receiver.dirty)){
  receiver.dirty=false;
  boundary.notify(receiver.id,receiver.value,origin);
  return true;
 }
 return false;
}
template<class Boundary>
std::optional<bool> set(preset_bank::Receiver& receiver,preset_bank::Value&& incoming,
 std::uint32_t origin,std::uint8_t mode,Boundary& boundary){
 return receiver.set_current(std::move(incoming),origin,mode,boundary);
}
}
