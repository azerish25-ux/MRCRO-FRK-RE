#include "mfrk/parameter_receiver.hpp"
#include "mfrk/parameter_dispatch.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <bit>
#include <fstream>
#include <iostream>
using namespace mfrk;
namespace mfrk::preset_bank {struct StoreTestAccess {static void observe(void(*fn)(const void*)){Value::free_observer_=fn;}};}
using preset_bank::Value;
static std::vector<const void*> freed;
static void on_free(const void* pointer){freed.push_back(pointer);}
struct Logical {std::uint32_t tag,size;std::array<std::uint8_t,32> hash;bool operator==(const Logical&)const=default;};
static Logical logical(const Value& value){return {value.tag(),std::uint32_t(value.bytes().size()),ui::scene::scope::test::sha256(value.bytes())};}
struct Notification {std::uint32_t origin,dirty;Logical value;bool operator==(const Notification&)const=default;};
struct Boundary {
 preset_bank::InternalReceiver& receiver;std::vector<Notification> events;
 void notify(std::uint32_t id,Value& value,std::uint32_t origin){if(id!=receiver.id||&value!=&receiver.value)throw std::runtime_error("shared receiver");events.push_back({origin,std::uint32_t(receiver.dirty),logical(value)});}
};
struct RecursiveBus {
 preset_bank::InternalReceiver receiver{0x80000017u,Value::scalar(0),false};parameter_dispatch::State state;parameter_dispatch::ListenerSet listeners{8,{1,2}};bool recursed=false;
 std::vector<std::array<std::uint32_t,4>> trace;
 const parameter_dispatch::ListenerSet* lookup(std::uint32_t id){return id==receiver.id?&listeners:nullptr;}
 void notify(std::uint32_t id,Value& value,std::uint32_t origin){if(!parameter_dispatch::dispatch(state,id,value,origin,*this))throw std::runtime_error("dispatch admission");}
 void invoke(std::uint64_t identity,std::uint32_t,Value& value,std::uint32_t origin){
  trace.push_back({std::uint32_t(identity),origin,value.scalar_bits(),std::uint32_t(state.dispatching)});
  if(!recursed){recursed=true;auto next=Value::scalar(0x3e800000u);parameter_receiver::set(receiver,std::move(next),7,2,*this);if(next.scalar_bits()!=0)throw std::runtime_error("nested move");}
 }
};
int main(int argc,char**argv)try{
 static_assert(std::endian::native==std::endian::little);if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=717812)return 2;
 std::vector<std::uint8_t> bytes(717812);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"IRECV001",8))return 2;
 if(ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x12,0x81,0xe4,0xc9,0xa5,0x81,0x55,0x04,0xcf,0x49,0xe6,0x8a,0x0b,0xf1,0x4c,0x15,0x8a,0x3e,0xfe,0x71,0x5d,0xfa,0xb9,0x9c,0x40,0xcb,0x21,0x6a,0x83,0x12,0xd7,0xe4})return 2;
 std::size_t position=8;auto read=[&]<class T>(){if(position>bytes.size()||sizeof(T)>bytes.size()-position)throw std::runtime_error("truncated");T result;std::memcpy(&result,bytes.data()+position,sizeof result);position+=sizeof result;return result;};
 auto value=[&](){const auto tag=read.operator()<std::uint8_t>();const auto size=read.operator()<std::uint16_t>();if(size>8192||position>bytes.size()||size>bytes.size()-position)throw std::runtime_error("value frame");auto data=std::span<const std::uint8_t>(bytes).subspan(position,size);position+=size;Value result;if(tag==1){if(size!=4)throw std::runtime_error("scalar size");std::uint32_t bits;std::memcpy(&bits,data.data(),4);result=Value::scalar(bits);}else result=Value::data(data);if(result.tag()!=tag)throw std::runtime_error("value tag");return result;};
 auto digest=[&](){const auto tag=read.operator()<std::uint8_t>();const auto size=read.operator()<std::uint16_t>();Logical result{tag,size,{}};for(auto& byte:result.hash)byte=read.operator()<std::uint8_t>();return result;};
 if(read.operator()<std::uint32_t>()!=860)return 2;
 unsigned differences=0,notifications=0;
 for(unsigned ordinal=0;ordinal<860;++ordinal){
  auto old=value(),incoming=value();const auto dirty=read.operator()<std::uint8_t>(),mode=read.operator()<std::uint8_t>();const auto origin=read.operator()<std::uint32_t>();const auto alias=read.operator()<std::uint8_t>();if(dirty>1||alias>1)return 2;
  const auto expected_value=digest(),expected_input=digest();const auto dirty_after=read.operator()<std::uint8_t>(),old_freed=read.operator()<std::uint8_t>(),transferred=read.operator()<std::uint8_t>(),count=read.operator()<std::uint8_t>();if(count>1)return 2;std::vector<Notification> expected_events;
  for(unsigned i=0;i<count;++i){const auto event_origin=read.operator()<std::uint32_t>();const auto event_dirty=read.operator()<std::uint8_t>();expected_events.push_back({event_origin,event_dirty,digest()});}
  preset_bank::InternalReceiver receiver{0x80000017u,std::move(old),bool(dirty)};Boundary boundary{receiver,{}};const auto* old_heap=receiver.value.tag()==2?receiver.value.data_address():nullptr;auto& input=alias?receiver.value:incoming;const auto* input_heap=input.tag()==2?input.data_address():nullptr;
  freed.clear();preset_bank::StoreTestAccess::observe(on_free);const bool notified=parameter_receiver::set(receiver,std::move(input),origin,mode,boundary);preset_bank::StoreTestAccess::observe(nullptr);
  const bool did_free=old_heap&&std::find(freed.begin(),freed.end(),old_heap)!=freed.end();const bool did_transfer=input_heap&&receiver.value.data_address()==input_heap;
  differences+=logical(receiver.value)!=expected_value||logical(input)!=expected_input||receiver.dirty!=bool(dirty_after)||boundary.events!=expected_events||did_free!=bool(old_freed)||did_transfer!=bool(transferred)||notified!=bool(count);notifications+=boundary.events.size();
 }
 if(position!=bytes.size()||notifications!=300)return 2;
 RecursiveBus bus;auto input=Value::scalar(0x3f000000u);if(!parameter_receiver::set(bus.receiver,std::move(input),4,1,bus))return 3;
 if(input.scalar_bits()!=0||bus.receiver.dirty||bus.state.dispatching||bus.trace!=std::vector<std::array<std::uint32_t,4>>{{1,4,0x3f000000u,1},{1,7,0x3e800000u,1},{2,7,0x3e800000u,1},{2,4,0x3e800000u,0}})return 3;
 std::cout<<"{\"cases\":860,\"notifications\":"<<notifications<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){preset_bank::StoreTestAccess::observe(nullptr);std::cerr<<error.what();return 2;}
