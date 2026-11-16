#include "mfrk/preset_bank.hpp"
#include "mfrk/preset_prepare.hpp"
#include <array>
using namespace mfrk;
using namespace mfrk::preset_bank;
int main(){
 std::vector<std::uint32_t> destroyed;BankPair banks;
 for(unsigned context=0;context<2;++context){auto& bank=context?banks.prepared():banks.active();bank.internal.push_back({0x80000000u,Value::scalar(context?6:1),false});const std::array<std::uint8_t,2> data{std::uint8_t(context?7:4),std::uint8_t(context?8:5)};bank.internal.push_back({0x80000001u,Value::data(data),false});bank.internal.push_back({0x80000002u,Value::data(std::vector<std::uint8_t>(4640,std::uint8_t(context?9:2))),false});if(!bank.positive.append(Receiver::make(0,Value::scalar(context?10:3),0,[&](auto id){destroyed.push_back(id);})))return 1;}
 const auto* old_current=banks.active().internal.data();const auto* old_prepared=banks.prepared().internal.data();auto held=Ref::retain(banks.prepared().positive.at(0));const auto* positive=held.get();
 const std::array<std::uint8_t,4> zero{};const std::array<std::uint8_t,2> empty_data{};
 const std::array<preset_program::Descriptor,3> internal{{{2,0,1,zero,"Foo"},{2,0,3,empty_data,"Blob"},{0,0,1,zero,"Unselected"}}};const std::array<preset_program::Descriptor,1> vst{{{2,0,1,zero,"Foo"}}};
 const std::string source="22 serialization::archive 10 0 7 0 7 4 Test 4 Bank 66 1 A 1 U 0 0 0 0 1 0 0  0 0  0 1 0 0 0 3 Key 5 Value 0 0 0 7 0 0 0 0 0 0 1 0 0 0 3 Foo -0 1 0 4 Blob 2 "+std::string("a\0",2);
 auto loaded=preset_prepare::load(preset_archive::Owned(std::as_bytes(std::span(source))),internal,vst,{});if(!loaded||!stage(banks.prepared(),loaded->prepared.writes))return 2;
 if(banks.active().internal.data()!=old_current||banks.prepared().internal.data()!=old_prepared||banks.active().internal[0].value.scalar_bits()!=1)return 3;
 banks.activate_main();if(banks.active().internal.data()!=old_prepared||banks.prepared().internal.data()!=old_current||banks.active().positive.at(0)!=positive||held->value().scalar_bits()!=0x80000000u)return 4;
 if(banks.active().internal[0].value.scalar_bits()!=0x80000000u||banks.active().internal[1].value.bytes()[0]!='a'||banks.active().internal[1].value.bytes()[1]!=0||banks.active().internal[2].value.bytes().size()!=4640||banks.active().internal[2].value.bytes().back()!=9)return 5;
 if(held->cached_bits()!=0||!held->dirty())return 6;
 const std::array<std::uint32_t,3> flags{2,2,0};const std::array<std::uint32_t,1> positive_flags{2};auto commands=notifications(banks.active(),flags,positive_flags);
 if(!commands||commands->size()!=3||(*commands)[0].wrapper_id!=0x80000000u||(*commands)[1].payload!=std::vector<std::uint8_t>{'a',0}||(*commands)[2].wrapper_id!=0)return 7;
 const preset_program::Write next{0x80000000u,1,{11,0,0,0},true,{}};const std::array writes{next};if(!stage(banks.prepared(),writes))return 8;
 banks.activate_main();if(banks.active().internal.data()!=old_current||banks.active().internal[0].value.scalar_bits()!=11||banks.active().internal[1].value.bytes()[0]!=4||banks.active().internal[2].value.bytes().back()!=2)return 9;
 const std::array duplicate{next,next};if(stage(banks.prepared(),duplicate)||banks.prepared().internal[0].value.scalar_bits()!=0x80000000u)return 10;
 // An externally retained receiver survives its container being cleared.
 banks.prepared().positive.clear();if(!destroyed.empty()||held->value().scalar_bits()!=0x80000000u||held->cached_bits()!=0)return 11;
 held.reset();if(destroyed!=std::vector<std::uint32_t>{0})return 12;
 // Its other bank owns a distinct receiver with the same wrapper ID.
 banks.active().positive.clear();if(destroyed!=std::vector<std::uint32_t>{0,0})return 13;
}
