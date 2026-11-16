#include "mfrk/wheels_listener.hpp"
#include "mfrk/preset_bank.hpp"
using namespace mfrk;
struct Boundary {
 bool suppressed=false;unsigned getters=0;std::vector<std::array<std::uint32_t,4>> writes;
 bool suppress(){++getters;return suppressed;}
 void write(std::uint32_t origin,std::uint32_t id,std::uint32_t bits,bool busy){writes.push_back({origin,id,bits,std::uint32_t(busy)});}
};
int main(){
 preset_bank::BankPair banks;for(auto* bank:{&banks.active(),&banks.prepared()})bank->internal.push_back({0x80000000u,preset_bank::Value::scalar(0),false});
 const std::array<preset_program::Write,1> writes{{{0x80000000u,1,{0,0,0,128},true,{}}}};if(!preset_bank::stage(banks.prepared(),writes))return 1;banks.activate_main();
 const std::array<std::uint32_t,1> flags{2};auto commands=preset_bank::notifications(banks.active(),flags,{});if(!commands||commands->size()!=1)return 2;
 wheels_listener::State state{{0x80000000u,0x80000001u,0x80000002u,0x80000003u},false};Boundary boundary;
 if(!wheels_listener::notify(state,(*commands)[0],boundary)||boundary.getters!=1||state.busy||boundary.writes!=std::vector<std::array<std::uint32_t,4>>{{4,0x80000001u,0x80000000u,1}})return 3;
 for(auto origin:{0u,3u}){boundary={};auto command=(*commands)[0];command.origin=origin;if(!wheels_listener::notify(state,command,boundary)||boundary.getters||!boundary.writes.empty())return 4;}
 boundary={};boundary.suppressed=true;if(!wheels_listener::notify(state,(*commands)[0],boundary)||boundary.getters!=1||state.busy||!boundary.writes.empty())return 5;
 boundary={};state.busy=true;if(!wheels_listener::notify(state,(*commands)[0],boundary)||boundary.getters!=1||!state.busy||!boundary.writes.empty())return 6;
 boundary={};state.busy=false;auto command=(*commands)[0];command.wrapper_id=99;if(!wheels_listener::notify(state,command,boundary)||boundary.getters!=1||state.busy||!boundary.writes.empty())return 7;
 boundary={};command.tag=3;if(wheels_listener::notify(state,command,boundary)||boundary.getters||state.busy)return 8;
}
