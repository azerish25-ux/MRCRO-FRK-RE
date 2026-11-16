#include "mfrk/ui_parameter_controller.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
namespace mfrk::preset_bank {struct StoreTestAccess {static void observe(void(*f)(const void*)){Value::free_observer_=f;}};}
namespace {
using Row=std::array<std::uint32_t,6>;
using namespace mfrk::ui;
std::vector<Row> events;
void freed(const void*){events.push_back({8,0,0,0,0,0});}
struct Observer:GuiParameterObserver {
 std::uint32_t index;
 explicit Observer(std::uint32_t i):index(i){}
 void changed(std::uint32_t id)override{events.push_back({7,index,0,id,0,0});}
};
struct Backend:GuiParameterBackend {
 GuiParameterState& state;const Row& input;std::uint32_t index;
 Backend* next=nullptr;Observer* first;Observer* second;
 Backend(GuiParameterState& s,const Row& c,std::uint32_t i,Observer& a,Observer& b):state(s),input(c),index(i),first(&a),second(&b){}
 void mutate(){if(input[3]&1)state={next,114,112,901,second};}
 void begin(std::uint32_t origin,std::uint32_t id)override{events.push_back({1,index,origin,id,0,0});mutate();}
 void end(std::uint32_t origin,std::uint32_t id)override{events.push_back({2,index,origin,id,0,0});mutate();}
 void write(std::uint32_t origin,std::uint32_t id,mfrk::preset_bank::Value& value)override{
  if(value.tag()!=1||value.bytes().size()!=4)throw std::runtime_error("Source scalar delivery");
  events.push_back({3,index,origin,id,value.scalar_bits(),4});
  if(input[5]){const std::array<std::uint8_t,24> bytes{};value=mfrk::preset_bank::Value::data(bytes);}
  mutate();
 }
 void changed(std::uint32_t id,std::uintptr_t context)override{
  events.push_back({4,index,0,id,static_cast<std::uint32_t>(context),0});
  if(input[3]&2){state.id=214;state.observer=first;}
 }
 void prepared(std::uint32_t id,std::uintptr_t context)override{events.push_back({5,index,0,id,static_cast<std::uint32_t>(context),0});}
 void ended(std::uint32_t id,std::uintptr_t context)override{events.push_back({6,index,0,id,static_cast<std::uint32_t>(context),0});}
};
}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 std::ifstream f(argv[1],std::ios::binary|std::ios::ate);if(!f||f.tellg()!=46956)return 2;
 std::vector<std::uint8_t> bytes(46956);f.seekg(0);if(!f.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))return 2;
 if(std::memcmp(bytes.data(),"GUIPARM1",8)||mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x57,0xd1,0xa6,0x0c,0x65,0xa5,0xbe,0xb1,0x1f,0x41,0x50,0x97,0x7e,0xa8,0xbb,0xac,0x5c,0xef,0xb9,0x62,0xbe,0xc2,0xbe,0x35,0x8e,0x7a,0x0a,0x6c,0xb6,0x1e,0x5b,0x8e})return 2;
 std::size_t at=8;auto read=[&](){if(at+4>bytes.size())throw std::runtime_error("Fixture bounds");std::uint32_t v;std::memcpy(&v,bytes.data()+at,4);at+=4;return v;};
 const auto count=read();unsigned failures=0;
 mfrk::preset_bank::StoreTestAccess::observe(freed);
 for(std::uint32_t i=0;i<count;++i){
  Row c;for(auto& v:c)v=read();const auto n=read();std::vector<Row> expected(n);for(auto& row:expected)for(auto& v:row)v=read();
  GuiParameterState state{};Observer first(0),second(1);Backend a(state,c,0,first,second),b(state,c,1,first,second);a.next=b.next=&b;
  state={&a,13,111,900,c[4]==2?nullptr:c[4]==0?&first:&second};GuiParameterController controller(state);events.clear();
  if(c[0]==0){if(!controller.prepare())throw std::runtime_error("prepare result");}
  else if(c[0]==1){if(!controller.end())throw std::runtime_error("end result");}
  else controller.set(std::bit_cast<float>(c[1]),c[2]!=0);
  if(events!=expected){if(failures++<4)std::cerr<<"Callback mismatch case "<<i<<'\n';}
 }
 mfrk::preset_bank::StoreTestAccess::observe(nullptr);
 if(at!=bytes.size())return 2;
 std::cout<<count<<" original GUI controller cases, "<<failures<<" differences\n";return failures?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
