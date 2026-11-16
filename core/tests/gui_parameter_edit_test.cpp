#include "mfrk/ui_parameter_controller.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
namespace {
using namespace mfrk::ui;
using Input=std::array<std::uint32_t,10>;
using Event=std::array<std::uint32_t,6>;
std::vector<Event> events;
struct Observer:GuiParameterObserver {void changed(std::uint32_t)override{throw std::logic_error("Edit observer notification");}void scalar_changed(std::uint32_t,float)override{throw std::logic_error("Edit scalar notification");}};
struct Mode:GuiParameterValue {
 const mfrk::preset_bank::Value* value=nullptr;
 const mfrk::preset_bank::Value* current()override{events.push_back({2,0,0,0,0,0});return value;}
};
struct Owner:GuiParameterOwner {
 GuiParameterState& state;const Input& input;GuiParameterBackend* next=nullptr;const GuiParameterDescriptor& descriptor;
 Owner(GuiParameterState& s,const Input& c,const GuiParameterDescriptor& d):state(s),input(c),descriptor(d){}
 bool selection_capability()override{
  events.push_back({5,0,0,0,0,0});
  if(input[8]){state.backend=next;state.id=214;state.context=902;state.descriptor=&descriptor;}
  return input[2]!=0;
 }
};
struct Backend:GuiParameterBackend {
 GuiParameterState& state;const Input& input;Observer& observer;const std::uint32_t& linked;Mode& mode;Owner& owner_value;std::uint32_t index;Backend* next=nullptr;
 Backend(GuiParameterState& s,const Input& c,Observer& o,const std::uint32_t& l,Mode& m,Owner& v,std::uint32_t i):state(s),input(c),observer(o),linked(l),mode(m),owner_value(v),index(i){}
 GuiParameterValue& parameter(std::uint32_t)override{throw std::logic_error("Edit fixture parameter lookup");}
 GuiParameterValue* mode_parameter(std::uint32_t id,std::uint32_t flags)override{
  events.push_back({1,index,id,flags,0,0});if(input[9])observer.linked_parameter=input[1]?nullptr:&linked;
  return input[0]==0?nullptr:&mode;
 }
 void select(std::uint32_t id,bool primary,bool secondary,std::uintptr_t context)override{
  events.push_back({3,index,id,primary,secondary,static_cast<std::uint32_t>(context)});
  if(input[7]){state.backend=next;state.id=114;state.context=901;}
 }
 GuiParameterOwner& owner()override{events.push_back({4,index,0,0,0,0});return owner_value;}
 std::optional<std::uint32_t> selection(std::uint32_t category,std::uint32_t flags)override{
  events.push_back({6,index,category,flags,0,0});if(input[3]==0)return std::nullopt;return state.id+(input[3]==2);
 }
 void begin(std::uint32_t,std::uint32_t)override{throw std::logic_error("Edit fixture begin");}
 void end(std::uint32_t,std::uint32_t)override{throw std::logic_error("Edit fixture end");}
 void write(std::uint32_t,std::uint32_t,mfrk::preset_bank::Value&)override{throw std::logic_error("Edit fixture write");}
 void changed(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Edit fixture notification");}
 void prepared(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Edit fixture notification");}
 void ended(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Edit fixture notification");}
};
}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 std::ifstream f(argv[1],std::ios::binary|std::ios::ate);if(!f||f.tellg()!=65580)return 2;
 std::vector<std::uint8_t> bytes(65580);f.seekg(0);if(!f.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))return 2;
 if(std::memcmp(bytes.data(),"GUIEDIT1",8)||mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xfa,0x5b,0x18,0xc1,0x5c,0x29,0x04,0xa4,0xa1,0xfb,0xc5,0xb8,0xaf,0x84,0x6c,0xe7,0x23,0xbf,0xdb,0x4a,0x36,0xfe,0x6d,0x24,0x36,0xed,0x91,0x75,0x26,0xe1,0x5c,0xe2})return 2;
 std::size_t at=8;auto read=[&](){if(at+4>bytes.size())throw std::runtime_error("Fixture bounds");std::uint32_t v;std::memcpy(&v,bytes.data()+at,4);at+=4;return v;};const auto count=read();unsigned failures=0;
 for(std::uint32_t i=0;i<count;++i){
  Input c;for(auto& v:c)v=read();const auto expected_result=read(),n=read();std::vector<Event> expected(n);for(auto& e:expected)for(auto& v:e)v=read();
  const std::array<std::uint32_t,7> bits{0,0,0x3f800000,0xbf800000,0x7fc12345,0,0};const std::array<std::uint8_t,8> data{};
  auto value=c[0]==5?mfrk::preset_bank::Value::data(data):mfrk::preset_bank::Value::scalar(bits[c[0]]);Mode mode;mode.value=c[0]==6?nullptr:&value;
  GuiParameterState state{};const std::uint32_t link=77;Observer observer;observer.linked_parameter=c[1]?&link:nullptr;
  GuiParameterDescriptor descriptor{0,c[4]==3?2ull:1ull,c[4]>=2?1u:0u,0},after_capability{0,1,1,0};Owner owner(state,c,after_capability);
  Backend a(state,c,observer,link,mode,owner,0),b(state,c,observer,link,mode,owner,1);a.next=b.next=&b;owner.next=&b;
  state={&a,13,111,900,&observer,c[4]==0?nullptr:&descriptor};GuiParameterController controller(state);events.clear();const bool result=controller.edit(c[5]!=0,c[6]!=0);
  if(static_cast<unsigned>(result)!=expected_result||events!=expected){if(failures++<4)std::cerr<<"Edit mismatch case "<<i<<'\n';}
 }
 if(at!=bytes.size())return 2;
 std::cout<<count<<" original GUI edit gates, "<<failures<<" differences\n";return failures?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
