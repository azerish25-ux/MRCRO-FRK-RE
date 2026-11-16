#include "mfrk/ui_write_dispatch.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
namespace {
using namespace mfrk;using namespace mfrk::ui;
using Input=std::array<std::uint32_t,8>;using Event=std::array<std::uint32_t,12>;
std::array<std::uint32_t,8> payload(const preset_bank::Value& value){
 std::array<std::uint32_t,8> out{value.tag(),static_cast<std::uint32_t>(value.bytes().size())};
 if(value.bytes().size()>24)throw std::logic_error("Fixture payload size");
 std::memcpy(out.data()+2,value.bytes().data(),value.bytes().size());return out;
}
struct Boundary:GuiWriteDispatchBoundary {
 const Input& input;std::vector<Event> events;std::unique_ptr<GuiWriteMessage> pending;
 explicit Boundary(const Input& c):input(c){}
 void event(std::uint32_t kind,std::uint32_t a=0,std::uint32_t b=0,std::uint32_t c=0,const preset_bank::Value* value=nullptr){
  Event out{kind,a,b,c};if(value){const auto data=payload(*value);std::copy(data.begin(),data.end(),out.begin()+4);}events.push_back(out);
 }
 std::optional<std::uint64_t> message_thread()override{return input[0]?std::optional<std::uint64_t>(111):std::nullopt;}
 std::uint32_t current_thread()override{event(1);return input[0]==1?111:222;}
 void post(std::unique_ptr<GuiWriteMessage> message)override{event(2,message->origin,message->id,0,&message->value);pending=std::move(message);}
 std::uint32_t part_count()override{return input[2];}
 void prepare_part(std::uint32_t part)override{event(3,part,0);}
 std::uintptr_t receiver(std::uint32_t id,std::uint32_t part)override{event(4,id,part,input[6]);return input[6]?99:0;}
 bool apply(std::uint32_t origin,std::uint32_t id,preset_bank::Value& value,std::uintptr_t receiver,std::uint32_t part)override{
  if(receiver!=(input[6]?99u:0u))throw std::logic_error("Receiver identity");
  event(5,origin,id,part,&value);if(input[5])value=preset_bank::Value::scalar(0);return input[7]!=0;
 }
};
}
int main(int argc,char** argv)try{
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file)return 2;const auto length=file.tellg();if(length<12||length>65536)return 2;
 std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"GWDS0002",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x41,0xfa,0xca,0xe7,0xdd,0xb1,0x66,0xba,0x18,0xc9,0xa7,0xb3,0x92,0x64,0xce,0x25,0x47,0xa9,0x5f,0x99,0x62,0x2e,0xd8,0x31,0x06,0x8c,0xbf,0xc8,0x49,0xb0,0xde,0xda})return 2;
 std::size_t at=8;auto word=[&](){if(at+4>bytes.size())throw std::runtime_error("Fixture bounds");std::uint32_t v;std::memcpy(&v,bytes.data()+at,4);at+=4;return v;};const auto cases=word();if(cases!=216)return 2;unsigned differences=0,event_count=0;
 for(unsigned i=0;i<cases;++i){
  Input input;for(auto& v:input)v=word();std::array<std::uint32_t,8> expected;for(auto& v:expected)v=word();const auto expected_result=word();std::vector<Event> expected_events(word());for(auto& e:expected_events)for(auto& v:e)v=word();
  const std::array<std::uint32_t,6> raw{0x3e800000,0xdeadbeef,3,4,5,6};auto value=input[4]==4?preset_bank::Value::scalar(raw[0]):preset_bank::Value::data(std::span(reinterpret_cast<const std::uint8_t*>(raw.data()),input[4]));Boundary boundary(input);
  const auto result=gui_dispatch_write(input[3],input[1],value,boundary);const auto expected_route=input[0]==2?GuiWriteRoute::queued:((input[1]&0x7fffffffu)/1000000u>=input[2]?GuiWriteRoute::invalid_part:GuiWriteRoute::applied);
  if(result.route!=expected_route||result.accepted!=(expected_result!=0)||payload(value)!=expected||boundary.events!=expected_events){if(differences++<5)std::cerr<<"Write dispatch mismatch "<<i<<'\n';}event_count+=expected_events.size();
 }
 if(at!=bytes.size()||event_count!=432)return 2;
 std::cout<<"{\"cases\":"<<cases<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
