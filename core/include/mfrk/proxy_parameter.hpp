#pragma once
#include "mfrk/preset_program.hpp"
#include <stdexcept>
namespace mfrk::proxy_parameter {
// Source 71c1a0 tag-1/binary32 alias listener. The ordered alias vector and
// owner remain stable throughout dispatch. Boundary.get returns raw binary32;
// Boundary.write receives (origin, targetID, original payload bits).
// Constructors, receiver coercion, cascading listeners and FP status are separate.
inline bool equal_or_unordered(std::uint32_t a,std::uint32_t b) noexcept {
 if((a&0x7fffffffu)>0x7f800000u||(b&0x7fffffffu)>0x7f800000u)return true;
 if((a&0x7fffffffu)==0&&(b&0x7fffffffu)==0)return true;
 return a==b;
}
template<class Boundary>
void notify(std::span<const std::uint32_t> aliases,std::uint32_t guard,
 std::uint32_t event,std::uint32_t payload,std::uint32_t origin,Boundary& boundary){
 if(aliases.size()>128)throw std::invalid_argument("proxy alias count");
 if(origin==guard)return;
 for(auto target:aliases){
  if(target==event)continue;
  if(equal_or_unordered(boundary.get(target),payload))continue;
  boundary.write(guard,target,payload);
 }
}
// Direct join from the preset bank's emitted typed command. Unsupported value
// types are rejected before any getter/write. This does not register a listener
// or execute the other native callback families.
template<class Boundary>
bool notify(const preset_program::Command& command,std::span<const std::uint32_t> aliases,
 std::uint32_t guard,Boundary& boundary){
 if(command.tag!=1||command.payload.size()!=4)return false;
 std::uint32_t bits=0;for(unsigned i=0;i<4;++i)bits|=std::uint32_t(command.payload[i])<<(8*i);
 notify(aliases,guard,command.wrapper_id,bits,command.origin,boundary);
 return true;
}
}
