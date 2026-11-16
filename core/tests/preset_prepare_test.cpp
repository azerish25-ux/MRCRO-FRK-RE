#include "mfrk/preset_prepare.hpp"
#include <array>
#include <bit>
using namespace mfrk;
int main(){
 const std::array<std::uint8_t,4> zero{};const std::array<std::string_view,2> old{"A","B"};const std::array<std::string_view,3> current{"B","A","C"};const std::array<preset::Record,2> records{{{{0,0,0},old},{{2,0,0},current}}};
 const std::array<preset_prepare::Versioned,1> versions{{{"Choice",{records,3,0.f},0,0}}};
 const std::array<preset_program::Descriptor,1> vst{{{2,0,1,zero,"Choice"}}};
 preset_prepare::State input;input.creator="1.0.0";input.word194=49;input.numeric={{"Choice",0},{"Unknown",123}};
 std::vector<std::uint8_t> blob{1,22,'s','e','r','i','a','l','i','z','a','t','i','o','n',':',':','a','r','c','h','i','v','e',1,10,0,0,0};
 const std::string path="Old/Single.raw";const auto base=blob.size();blob.resize(base+256,0);for(unsigned i=0;i<path.size();++i)blob[base+i]=path[i];blob.insert(blob.end(),{1,static_cast<std::uint8_t>(path.size()),0,0});input.data["Osc1WavetableObject"]=blob;
 auto prepared=preset_prepare::prepare(input,{},vst,versions);if(!prepared||!prepared->custom_changed||prepared->migrated.creator!="5.1.1.6945"||prepared->migrated.word194!=49||prepared->migrated.numeric.at("Choice")!=0x3f000000u||prepared->migrated.numeric.at("Unknown")!=123)return 1;
 const auto& migrated=prepared->migrated.data.at("Osc1WavetableObject");const std::string expected="Factory/WT/Factory/Single.raw";for(unsigned i=0;i<expected.size();++i)if(migrated[29+i]!=static_cast<std::uint8_t>(expected[i]))return 2;if(migrated[29+expected.size()]!=0)return 2;
 if(prepared->writes.size()!=1||prepared->writes[0].payload!=std::vector<std::uint8_t>{0,0,0,63})return 3;
 if(input.creator!="1.0.0"||input.data.at("Osc1WavetableObject")!=blob)return 4;
 input.creator="5.1.1.6945";auto held=preset_prepare::prepare(input,{},vst,versions);if(!held||held->custom_changed||held->migrated.data.at("Osc1WavetableObject")!=blob)return 5;
 input.data["Osc1WavetableObject"]={};auto erased=preset_prepare::prepare(input,{},vst,versions);if(!erased||!erased->custom_changed||erased->migrated.data.contains("Osc1WavetableObject"))return 6;
 input.data["Osc1WavetableObject"]={1,2,3};if(preset_prepare::prepare(input,{},vst,versions))return 7;
 preset_prepare::State empty;empty.creator="";auto init=preset_prepare::prepare(empty,{},vst,versions);if(!init||!init->migrated.numeric.empty()||init->writes.size()!=1)return 8;
}
