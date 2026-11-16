#include "mfrk/ui_parameter_controller.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
namespace {
using namespace mfrk::ui;
using Event=std::array<std::uint32_t,3>;
std::vector<Event> events;
struct Parameter:GuiParameterValue {
 const mfrk::preset_bank::Value* value=nullptr;std::uint32_t index=0;
 const mfrk::preset_bank::Value* current()override{events.push_back({2,index,0});return value;}
};
struct Backend:GuiParameterBackend {
 GuiParameterState& state;Parameter& result;Backend* other=nullptr;bool mutate;std::uint32_t index;
 Backend(GuiParameterState& s,Parameter& p,bool m,std::uint32_t i):state(s),result(p),mutate(m),index(i){}
 GuiParameterValue& parameter(std::uint32_t id)override{events.push_back({1,index,id});if(mutate){state.backend=other;state.id=114;}return result;}
 GuiParameterValue* mode_parameter(std::uint32_t,std::uint32_t)override{throw std::logic_error("Read fixture edit service");}
 void select(std::uint32_t,bool,bool,std::uintptr_t)override{throw std::logic_error("Read fixture edit service");}
 GuiParameterOwner& owner()override{throw std::logic_error("Read fixture edit service");}
 std::optional<std::uint32_t> selection(std::uint32_t,std::uint32_t)override{throw std::logic_error("Read fixture edit service");}
 void begin(std::uint32_t,std::uint32_t)override{throw std::logic_error("Read fixture write");}
 void end(std::uint32_t,std::uint32_t)override{throw std::logic_error("Read fixture write");}
 void write(std::uint32_t,std::uint32_t,mfrk::preset_bank::Value&)override{throw std::logic_error("Read fixture write");}
 void changed(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Read fixture notification");}
 void prepared(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Read fixture notification");}
 void ended(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Read fixture notification");}
};
}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 std::ifstream f(argv[1],std::ios::binary|std::ios::ate);if(!f||f.tellg()!=10764)return 2;
 std::vector<std::uint8_t> bytes(10764);f.seekg(0);if(!f.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))return 2;
 if(std::memcmp(bytes.data(),"GUIREAD1",8)||mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x47,0x06,0x94,0xa3,0xf3,0xa0,0xe9,0x07,0x01,0x32,0xfc,0xed,0x97,0xc1,0x40,0xde,0xf3,0xe8,0xe9,0xc9,0x29,0xc6,0xc9,0x6f,0xfb,0xbc,0xc9,0x07,0x15,0x96,0xd6,0x23})return 2;
 std::size_t at=8;auto read=[&](){if(at+4>bytes.size())throw std::runtime_error("Fixture bounds");std::uint32_t v;std::memcpy(&v,bytes.data()+at,4);at+=4;return v;};const auto count=read();unsigned failures=0;
 for(std::uint32_t i=0;i<count;++i){
  std::array<std::uint32_t,5> input;for(auto& v:input)v=read();const auto expected_bits=read();std::vector<Event> expected(2);for(auto& e:expected)for(auto& v:e)v=read();
  const std::array<std::uint8_t,24> data{};auto value=input[1]==1?mfrk::preset_bank::Value::scalar(input[0]):mfrk::preset_bank::Value::data(std::span(data).first(input[1]==2?24:8));
  Parameter parameter;parameter.value=input[1]==0?nullptr:&value;parameter.index=input[3];GuiParameterState state{};
  Backend first(state,parameter,input[4]!=0,0),second(state,parameter,false,1);first.other=&second;
  state={&first,input[2],111,900,nullptr};GuiParameterController controller(state);events.clear();const auto result=std::bit_cast<std::uint32_t>(controller.current());
  if(result!=expected_bits||events!=expected){if(failures++<4)std::cerr<<"Read mismatch case "<<i<<'\n';}
 }
 if(at!=bytes.size())return 2;
 std::cout<<count<<" original GUI parameter reads, "<<failures<<" differences\n";return failures?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
