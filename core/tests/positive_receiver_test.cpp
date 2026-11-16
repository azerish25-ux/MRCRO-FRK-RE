#include "mfrk/parameter_receiver.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
using namespace mfrk;
namespace mfrk::preset_bank {struct StoreTestAccess {
 static void dirty(Receiver& receiver,bool value){receiver.dirty_=value;}
 static Value& value(Receiver& receiver){return receiver.value_;}
};}
struct Event {std::uint32_t kind,arg,dirty,bits;std::uint64_t cache;bool operator==(const Event&)const=default;};
struct Boundary {
 preset_bank::Receiver& receiver;std::vector<Event> events;
 void changed(preset_bank::Receiver& owner,std::uint32_t message){if(&owner!=&receiver)throw std::runtime_error("changed owner");events.push_back({0,message,std::uint32_t(owner.dirty()),owner.value().scalar_bits(),owner.cached_bits()});}
 void notify(std::uint32_t id,preset_bank::Value& value,std::uint32_t origin){if(id!=receiver.id()||&value!=&receiver.value())throw std::runtime_error("notify owner");events.push_back({1,origin,std::uint32_t(receiver.dirty()),value.scalar_bits(),receiver.cached_bits()});}
};
int main(int argc,char**argv)try{
 static_assert(std::endian::native==std::endian::little);if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=674144)return 2;
 std::vector<std::uint8_t> bytes(674144);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"PRECV001",8))return 2;
 if(ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xfe,0xac,0x1a,0x2b,0xc0,0x6f,0xf2,0xf8,0xdf,0x72,0xba,0x7c,0x8f,0xc5,0xb7,0xe6,0xfa,0x52,0x6f,0xfd,0xac,0x89,0xe9,0x08,0x8c,0x31,0xbb,0xd8,0x3f,0x2c,0xe0,0xbb})return 2;
 std::size_t position=8;auto read=[&]<class T>(){if(position>bytes.size()||sizeof(T)>bytes.size()-position)throw std::runtime_error("truncated");T result;std::memcpy(&result,bytes.data()+position,sizeof result);position+=sizeof result;return result;};
 if(read.operator()<std::uint32_t>()!=11620)return 2;
 unsigned differences=0,changed=0,notified=0;
 for(unsigned ordinal=0;ordinal<11620;++ordinal){
  const auto old=read.operator()<std::uint32_t>(),bits=read.operator()<std::uint32_t>();const auto cache=read.operator()<std::uint64_t>();const auto origin=read.operator()<std::uint32_t>();const auto mode=read.operator()<std::uint8_t>(),dirty=read.operator()<std::uint8_t>(),alias=read.operator()<std::uint8_t>();
  const auto value_after=read.operator()<std::uint32_t>(),input_after=read.operator()<std::uint32_t>();const auto cache_after=read.operator()<std::uint64_t>();const auto dirty_after=read.operator()<std::uint8_t>(),count=read.operator()<std::uint8_t>();if(dirty>1||alias>1||dirty_after>1||count>2)return 2;
  std::vector<Event> expected;bool expected_notify=false;
  for(unsigned i=0;i<count;++i){const auto kind=read.operator()<std::uint8_t>();const auto arg=read.operator()<std::uint32_t>();const auto event_dirty=read.operator()<std::uint8_t>();const auto event_bits=read.operator()<std::uint32_t>();const auto event_cache=read.operator()<std::uint64_t>();if(kind>1||event_dirty>1)return 2;expected.push_back({kind,arg,event_dirty,event_bits,event_cache});expected_notify|=kind==1;}
  auto owner=preset_bank::Receiver::make(23,preset_bank::Value::scalar(old),cache);preset_bank::StoreTestAccess::dirty(*owner.get(),dirty);auto incoming=preset_bank::Value::scalar(bits);auto& input=alias?preset_bank::StoreTestAccess::value(*owner.get()):incoming;Boundary boundary{*owner.get(),{}};
  const auto result=parameter_receiver::set(*owner.get(),std::move(input),origin,mode,boundary);
  differences+=!result||(result&&*result!=expected_notify)||owner->value().scalar_bits()!=value_after||input.scalar_bits()!=input_after||owner->cached_bits()!=cache_after||owner->dirty()!=bool(dirty_after)||boundary.events!=expected;
  for(const auto& event:boundary.events)if(event.kind==0)++changed;else++notified;
 }
 if(position!=bytes.size()||changed!=4714||notified!=6270)return 2;
 // The same native cache rule applies while a persistent bank is prepared.
 preset_bank::Bank bank;auto owner=preset_bank::Receiver::make(0,preset_bank::Value::scalar(0x3f000000u),0x7ff0123456789abcull);if(!bank.positive.append(std::move(owner)))return 3;
 const std::array<preset_program::Write,1> writes{{{0,1,{0,0,64,63},true,0x3fe8000000000000ull}}};if(!preset_bank::stage(bank,writes)||bank.positive.at(0)->cached_bits()!=0x7ff0123456789abcull||bank.positive.at(0)->value().scalar_bits()!=0x3f400000u)return 3;
 const std::array<std::uint32_t,1> flags{2};auto commands=preset_bank::notifications(bank,{},flags);if(!commands||commands->size()!=1||(*commands)[0].payload!=writes[0].payload)return 3;
 Boundary boundary{*bank.positive.at(0),{}};const std::array<std::uint8_t,2> data{1,2};auto unsupported=preset_bank::Value::data(data);if(parameter_receiver::set(*bank.positive.at(0),std::move(unsupported),4,1,boundary)||unsupported.tag()!=3||!boundary.events.empty())return 3;
 std::cout<<"{\"cases\":11620,\"changed_events\":"<<changed<<",\"notifications\":"<<notified<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what();return 2;}
