#pragma once
#include <cstdint>
#include <optional>
#include <vector>
namespace mfrk::parameter_dispatch {
// Current native unordered listener-set iteration and capacity, before dispatch.
// Identities are opaque native-address-equivalent tokens, not owned objects.
// Supplied max-load-factor-1 profile: 8..65536 power-of-two buckets, <=16384 unique
// nonzero identities, count <= buckets. These are adapter admissions.
struct ListenerSet {std::uint32_t buckets=8;std::vector<std::uint64_t> iteration;};
std::optional<std::vector<std::uint64_t>> snapshot(const ListenerSet&);
struct State {bool dispatching=false;};
// Source 6d16a0: mark busy before exact-key lookup, clone the native set, then
// invoke that clone. Boundary.lookup(key) supplies the current set or nullptr;
// Boundary.invoke(identity,key,payload,origin) supplies the actual listener.
// The payload is shared by reference, never frozen in the snapshot. A nested
// dispatch clears the same boolean on return; this is not a nesting counter.
// Serial stable source input during cloning, live listener objects and no
// exceptions are required. Registry/ownership, callbacks and bank transfer are
// separate. A false result means an unsupported supplied set was rejected.
template<class Payload,class Boundary>
bool dispatch(State& state,std::uint32_t key,Payload& payload,std::uint32_t origin,Boundary& boundary){
 state.dispatching=true;
 if(const auto* source=boundary.lookup(key)){
  auto copied=snapshot(*source);if(!copied){state.dispatching=false;return false;}
  for(auto identity:*copied)boundary.invoke(identity,key,payload,origin);
 }
 state.dispatching=false;
 return true;
}
}
