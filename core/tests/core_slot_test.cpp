#include "mfrk/core_bank.hpp"
#include <array>
#include <iostream>
#include <memory>

int main(){
 using namespace mfrk::core_bank;
 auto tables=std::make_unique<Tables>();
 mfrk::karplus::SharedRandom random{54321};
 auto side=std::make_unique<Side>(*tables,random,0x400020000ull);
 SlotController slot(*side);
 if(!slot.select(Model::basic)||slot.dirty()!=std::array<std::uint8_t,3>{})return 1;
 slot.phase(.25f);
 if(!slot.select(Model::karplus)||slot.selected()!=Model::karplus||side->active(Model::basic)||!side->active(Model::karplus)||slot.dirty()!=std::array<std::uint8_t,3>{1,1,1})return 2;
 slot.clear_dirty();
 if(!slot.select(Model::karplus)||slot.dirty()!=std::array<std::uint8_t,3>{})return 3;
 if(!slot.select(Model::harmo)||side->active(Model::karplus)||!side->active(Model::harmo))return 4;
 if(!slot.select(Model::basic)||side->active(Model::harmo)||!side->active(Model::basic))return 5;
 if(slot.select(static_cast<Model>(255)))return 6;
 std::cout<<"slot selection profile passed\n";
}
