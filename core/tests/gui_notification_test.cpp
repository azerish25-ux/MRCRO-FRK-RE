#include "mfrk/ui_parameter_notification.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
namespace {
using namespace mfrk::ui;
using Input=std::array<std::uint32_t,21>;using Event=std::array<std::uint32_t,12>;
struct Test;
struct Parameter:GuiParameterValue {Test* test=nullptr;unsigned index=0;mfrk::preset_bank::Value value;const mfrk::preset_bank::Value* current()override;};
struct Observer:GuiParameterObserver {Test& test;explicit Observer(Test& t):test(t){}void changed(std::uint32_t)override;void scalar_changed(std::uint32_t,float)override;};
struct Backend:GuiParameterBackend {
 Test& test;explicit Backend(Test& t):test(t){}
 GuiParameterValue& parameter(std::uint32_t)override{throw std::logic_error("Callback parameter lookup");}
 GuiParameterValue* mode_parameter(std::uint32_t,std::uint32_t)override;
 void select(std::uint32_t,bool,bool,std::uintptr_t)override{throw std::logic_error("Callback select");}
 GuiParameterOwner& owner()override{throw std::logic_error("Callback owner");}
 std::optional<std::uint32_t> selection(std::uint32_t,std::uint32_t)override;
 void begin(std::uint32_t,std::uint32_t)override{throw std::logic_error("Callback begin");}
 void end(std::uint32_t,std::uint32_t)override{throw std::logic_error("Callback end");}
 void write(std::uint32_t,std::uint32_t,mfrk::preset_bank::Value&)override;
 void changed(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Callback backend changed");}
 void prepared(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Callback prepared");}
 void ended(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Callback ended");}
};
struct Boundary:GuiNotificationBoundary {
 Test& test;explicit Boundary(Test& t):test(t){}
 std::uint32_t parameter_id(GuiParameterValue&)override;
 GuiParameterValue& feature_parameter(GuiParameterBackend&,std::uint32_t)override;
 std::optional<std::uint32_t> mapped_id(std::uint32_t)override;
 bool mappable(std::uint32_t)override;
 std::uint32_t active_id()override;
 bool assignment_allowed(std::uint32_t)override;
 std::uint32_t context_selected_id()override;
 void invalidate(std::uintptr_t)override;
};
void initialize(GuiControlInfo& info,unsigned config,unsigned flags,std::uintptr_t view){
 std::array<bool*,4> configuration{&info.flag08,&info.flag09,&info.flag0a,&info.flag0d};for(unsigned i=0;i<4;++i)*configuration[i]=(config&(1u<<i))!=0;
 std::array<bool*,8> values{&info.flag0b,&info.flag0c,&info.flag0e,&info.flag20,&info.flag21,&info.flag22,&info.flag23,&info.flag24};for(unsigned i=0;i<8;++i)*values[i]=(flags&(1u<<i))!=0;
 info.view=view;info.midi28=7;info.midi2c=8;
}
struct Test {
 const Input& input;std::vector<Event> events;unsigned writes=0;GuiParameterDescriptor descriptor{};std::array<GuiControlInfo,2> info{};std::array<Parameter,5> parameters;Observer observer;Backend backend;Boundary boundary;GuiParameterState state;
 explicit Test(const Input& c):input(c),observer(*this),backend(*this),boundary(*this),state{&backend,13,73,0,&observer,&descriptor,c[3]?&info[0]:nullptr}{
  descriptor.flags20=c[2];observer.linked_parameter=c[17]?&input[18]:nullptr;
  for(unsigned i=0;i<3;++i)observer.aliases.push_back({(c[19]&(1u<<i))!=0,std::array<std::uint32_t,3>{13,17,15}[i]});
  initialize(info[0],c[4],c[5],c[6]?99:0);initialize(info[1],c[4]^15,c[5]^0xa5,c[6]?100:0);
  for(unsigned i=0;i<5;++i){parameters[i].test=this;parameters[i].index=i;parameters[i].value=mfrk::preset_bank::Value::scalar(i==4?c[10]:i==0&&c[8]?0x3f800000u:0);}
 }
 void event(std::uint32_t kind,std::initializer_list<std::uint32_t> values={}){Event e{};e[0]=kind;std::copy(values.begin(),values.end(),e.begin()+1);events.push_back(e);}
 std::vector<std::uint32_t> snapshot()const{
  std::vector<std::uint32_t> out{state.id,state.origin,state.control_info==&info[1]?1u:0u,state.control_info?1u:0u};
  for(const auto& v:info){for(bool b:std::array<bool,12>{v.flag08,v.flag09,v.flag0a,v.flag0b,v.flag0c,v.flag0d,v.flag0e,v.flag20,v.flag21,v.flag22,v.flag23,v.flag24})out.push_back(b);out.push_back(std::bit_cast<std::uint32_t>(v.midi28));out.push_back(std::bit_cast<std::uint32_t>(v.midi2c));}
  out.push_back(static_cast<std::uint32_t>(observer.aliases.size()));for(unsigned i=0;i<4;++i)out.push_back(i<observer.aliases.size()&&observer.aliases[i].enabled);return out;
 }
};
const mfrk::preset_bank::Value* Parameter::current(){test->event(5,{index});return &value;}
void Observer::changed(std::uint32_t id){test.event(11,{id});}
void Observer::scalar_changed(std::uint32_t id,float value){test.event(12,{id,std::bit_cast<std::uint32_t>(value)});}
GuiParameterValue* Backend::mode_parameter(std::uint32_t category,std::uint32_t){
 test.event(2,{category});const std::array<std::uint32_t,4> categories{11,14,15,42};const auto at=std::find(categories.begin(),categories.end(),category);if(at==categories.end())throw std::logic_error("Callback property");const auto i=static_cast<unsigned>(at-categories.begin());return test.input[7]&(1u<<i)?&test.parameters[i]:nullptr;
}
std::optional<std::uint32_t> Backend::selection(std::uint32_t category,std::uint32_t){test.event(3,{category});return test.input[9]&(1u<<(category-2))?std::optional(100+category):std::nullopt;}
void Backend::write(std::uint32_t origin,std::uint32_t id,mfrk::preset_bank::Value& value){
 Event e{13,origin,id,value.tag(),static_cast<std::uint32_t>(value.bytes().size())};if(value.bytes().size()>24)throw std::logic_error("Callback write size");for(unsigned i=0;i<value.bytes().size()/4;++i)std::memcpy(&e[5+i],value.bytes().data()+i*4,4);test.events.push_back(e);++test.writes;
 if(test.input[20]&4)value=mfrk::preset_bank::Value::scalar(0);
 if(test.input[20]&8)test.state.id=14;
 if((test.input[20]&16)&&test.writes==1){test.observer.aliases[1].enabled=true;test.observer.aliases.push_back({true,19});}
}
std::uint32_t Boundary::parameter_id(GuiParameterValue& value){const auto index=static_cast<Parameter&>(value).index;test.event(6,{index});return std::array<std::uint32_t,4>{211,214,215,242}[index];}
GuiParameterValue& Boundary::feature_parameter(GuiParameterBackend&,std::uint32_t category){test.event(4,{category});return test.parameters[4];}
std::optional<std::uint32_t> Boundary::mapped_id(std::uint32_t id){test.event(1,{id});return test.input[11]?std::optional(test.input[12]):std::nullopt;}
bool Boundary::mappable(std::uint32_t id){test.event(7,{id});return test.input[13]!=0;}
std::uint32_t Boundary::active_id(){return test.input[14];}
bool Boundary::assignment_allowed(std::uint32_t id){test.event(8,{id});return test.input[15]!=0;}
std::uint32_t Boundary::context_selected_id(){test.event(9);return test.input[16];}
void Boundary::invalidate(std::uintptr_t view){test.event(10,{static_cast<std::uint32_t>(view)});if(test.input[20]&1)test.state.id=14;if(test.input[20]&2)test.state.control_info=&test.info[1];}
}
int main(int argc,char** argv)try{
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream f(argv[1],std::ios::binary|std::ios::ate);if(!f||f.tellg()!=372936)return 2;std::vector<std::uint8_t> bytes(372936);f.seekg(0);if(!f.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"GUINOT01",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xe1,0x4b,0x4b,0x90,0xf3,0xc1,0x2c,0xa7,0x9e,0xef,0x9a,0x2b,0x27,0x25,0x6f,0x0b,0x66,0x8d,0x4f,0xb9,0xeb,0x1b,0x66,0x07,0x4a,0x10,0xb9,0xc8,0x17,0xab,0xaa,0x60})return 2;
 std::size_t at=8;auto word=[&](){if(at+4>bytes.size())throw std::runtime_error("Fixture bounds");std::uint32_t value;std::memcpy(&value,bytes.data()+at,4);at+=4;return value;};const auto cases=word();if(cases!=640||word()!=21||word()!=37||word()!=12)return 2;unsigned differences=0,event_count=0;
 for(unsigned i=0;i<cases;++i){
  Input input;for(auto& v:input)v=word();const auto count=word();std::array<std::uint32_t,6> payload_words;for(auto& v:payload_words)v=word();if(count!=1&&count!=2&&count!=6)return 2;std::vector<std::uint32_t> expected_state;for(unsigned j=0;j<37;++j)expected_state.push_back(word());const auto n=word();std::vector<Event> expected_events(n);for(auto& e:expected_events)for(auto& v:e)v=word();
  auto value=count==1?mfrk::preset_bank::Value::scalar(payload_words[0]):mfrk::preset_bank::Value::data(std::span(reinterpret_cast<const std::uint8_t*>(payload_words.data()),count*4));Test test(input);GuiParameterNotification notification(test.state,test.boundary);notification.notify(input[0],value,input[1]);
  if(test.snapshot()!=expected_state||test.events!=expected_events){if(differences++<6){std::cerr<<"GUI notification mismatch "<<i<<" state="<<(test.snapshot()!=expected_state)<<'\n';for(unsigned j=0;j<std::min(test.events.size(),expected_events.size());++j)if(test.events[j]!=expected_events[j]){std::cerr<<" first event "<<j<<" actual "<<test.events[j][0]<<" expected "<<expected_events[j][0]<<'\n';break;}}}
  event_count+=n;
 }
 if(at!=bytes.size()||event_count!=4249)return 2;
 std::cout<<"{\"cases\":"<<cases<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
