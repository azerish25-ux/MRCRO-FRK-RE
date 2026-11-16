#include "mfrk/ui_gui_registration.hpp"
#include "mfrk/ui_bank_parameter.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
namespace {
using namespace mfrk;using namespace mfrk::ui;
using Input=std::array<std::uint32_t,13>;using Event=std::array<std::uint32_t,4>;
struct Backend:GuiParameterBackend {
 GuiParameterValue& parameter(std::uint32_t)override{throw std::logic_error("Registration value read");}
 GuiParameterValue* mode_parameter(std::uint32_t,std::uint32_t)override{throw std::logic_error("Registration property read");}
 void select(std::uint32_t,bool,bool,std::uintptr_t)override{throw std::logic_error("Registration selection");}
 GuiParameterOwner& owner()override{throw std::logic_error("Registration owner");}
 std::optional<std::uint32_t> selection(std::uint32_t,std::uint32_t)override{throw std::logic_error("Registration feature read");}
 void begin(std::uint32_t,std::uint32_t)override{throw std::logic_error("Registration begin");}
 void end(std::uint32_t,std::uint32_t)override{throw std::logic_error("Registration end");}
 void write(std::uint32_t,std::uint32_t,preset_bank::Value&)override{throw std::logic_error("Registration write");}
 void changed(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Registration changed");}
 void prepared(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Registration prepared");}
 void ended(std::uint32_t,std::uintptr_t)override{throw std::logic_error("Registration ended");}
};
struct Observer:GuiParameterObserver {void changed(std::uint32_t)override{throw std::logic_error("Registration observer");}void scalar_changed(std::uint32_t,float)override{throw std::logic_error("Registration scalar");}};
struct Notification:GuiNotificationBoundary {
 std::uint32_t parameter_id(GuiParameterValue&)override{throw std::logic_error("Registration ID");}
 GuiParameterValue& feature_parameter(GuiParameterBackend&,std::uint32_t)override{throw std::logic_error("Registration parameter");}
 std::optional<std::uint32_t> mapped_id(std::uint32_t)override{throw std::logic_error("Registration mapped");}
 bool mappable(std::uint32_t)override{throw std::logic_error("Registration map");}
 std::uint32_t active_id()override{throw std::logic_error("Registration active");}
 bool assignment_allowed(std::uint32_t)override{throw std::logic_error("Registration assignment");}
 std::uint32_t context_selected_id()override{throw std::logic_error("Registration context");}
 void invalidate(std::uintptr_t)override{throw std::logic_error("Registration invalidation");}
};
struct Context:GuiRegistrationContext {
 std::uint32_t mode=0;std::array<preset_bank::BankPair,3> banks;
 Context(){for(auto* bank:{&banks[0].active(),&banks[0].prepared()}){
  for(unsigned i=0;i<1300;++i)bank->internal.push_back({0x80000000u|i,preset_bank::Value::scalar(0),false});
  for(unsigned i=0;i<1300;++i)if(!bank->positive.append(preset_bank::Receiver::make(i,preset_bank::Value::scalar(0),0)))throw std::logic_error("Registration bank setup");
 }}
 std::uint32_t context(GuiParameterBackend&)override{return mode;}
 void prepare_part(GuiParameterBackend&,std::uint32_t part,std::uint32_t selected)override{if(part>=banks.size()||selected>1)throw std::logic_error("Registration part context");}
 bool receiver_exists(GuiParameterBackend&,std::uint32_t id,std::uint32_t selected)override{const auto group=(id&0x7fffffffu)/1000000u;if(group>=banks.size()||selected>1)return false;auto& bank=selected?banks[group].prepared():banks[group].active();return GuiBankParameterView::resolve(bank,id-group*1000000u).has_value();}
};
struct Trace:GuiRegistrationBoundary {
 GuiNotifierRegistration& concrete;GuiParameterDescriptor& descriptor;std::uint32_t mutation;std::vector<Event> events;
 Trace(GuiNotifierRegistration& c,GuiParameterDescriptor& d,std::uint32_t m):concrete(c),descriptor(d),mutation(m){}
 std::uint32_t register_controller(GuiParameterBackend& b,std::uint32_t id,parameter_dispatch::ListenerRegistry& r)override{events.push_back({1,id,0,0});return concrete.register_controller(b,id,r);}
 void unregister_controller(GuiParameterBackend& b,std::uint32_t id,parameter_dispatch::ListenerRegistry& r)override{events.push_back({2,id,0,0});concrete.unregister_controller(b,id,r);}
 bool feature(GuiParameterBackend& b,std::uint32_t kind,std::uint32_t part,parameter_dispatch::ListenerRegistry& r,bool subscribe)override{events.push_back({3,kind,part,subscribe});const bool result=concrete.feature(b,kind,part,r,subscribe);if(kind==3&&(mutation&1))descriptor.flags204^=1u<<16;return result;}
 bool property(GuiParameterBackend& b,std::uint32_t kind,std::uint32_t part,parameter_dispatch::ListenerRegistry& r,bool subscribe)override{events.push_back({4,kind,part,subscribe});const bool result=concrete.property(b,kind,part,r,subscribe);if(kind==11&&(mutation&2))descriptor.flags204^=1u<<18;return result;}
};
template<class T>void append(std::vector<std::uint8_t>& out,T value){const auto p=out.size();out.resize(p+sizeof(T));std::memcpy(out.data()+p,&value,sizeof(T));}
std::array<std::uint8_t,32> digest(GuiRegisteredParameter& owner,parameter_dispatch::ParameterNotifier& notifier,const std::vector<std::uint32_t>& keys,std::uint32_t counter,Context& context,GuiParameterDescriptor& descriptor){
 std::vector<std::uint8_t> data;append(data,owner.state().id);append(data,owner.state().origin);append(data,counter);append(data,context.mode);
 for(unsigned slot=0;slot<6;++slot){std::vector<std::pair<std::uint32_t,const parameter_dispatch::ListenerSet*>> rows;for(auto key:keys)if((key&0x7fffffffu)/1000000u==slot%3)if(const auto* set=notifier.members(key,slot>=3))rows.emplace_back(key,set);append(data,static_cast<std::uint32_t>(rows.size()));for(const auto& [key,set]:rows){append(data,key);append(data,static_cast<std::uint32_t>(set->iteration.size()));for(auto identity:set->iteration)append(data,identity);}}
 auto& registry=owner.listener();append(data,static_cast<std::uint32_t>(registry.bucket_count()));append(data,std::uint32_t(registry.destroying()));append(data,static_cast<std::uint32_t>(registry.size()));for(const auto& row:registry.registrations()){append(data,row.id);append(data,row.notifier);}
 append(data,descriptor.flags20);append(data,static_cast<std::uint32_t>(descriptor.count48));append(data,descriptor.flagsd0);append(data,descriptor.flags204);return mfrk::ui::scene::scope::test::sha256(data);
}
}
int main(int argc,char** argv)try{
 static_assert(std::endian::native==std::endian::little);
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=54208)return 2;
 std::vector<std::uint8_t> bytes(54208);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size())||std::memcmp(bytes.data(),"GUIREG01",8))return 2;
 if(mfrk::ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x32,0x39,0xb4,0x6d,0x86,0x09,0x3d,0xc3,0xab,0xe6,0x34,0xf7,0x08,0x65,0xc7,0xd3,0x2b,0xdb,0x63,0x45,0xa3,0x6f,0xf6,0x2d,0x6d,0x42,0xb0,0xf7,0xd1,0xca,0xaa,0xd2})return 2;
 std::size_t at=8;auto read=[&]<class T>(){if(at+sizeof(T)>bytes.size())throw std::runtime_error("Fixture bounds");T value;std::memcpy(&value,bytes.data()+at,sizeof(T));at+=sizeof(T);return value;};auto u32=[&](){return read.operator()<std::uint32_t>();};const auto cases=u32();if(cases!=192||u32()!=13)return 2;unsigned differences=0,events=0;
 for(unsigned i=0;i<cases;++i){
  Input c;for(auto& v:c)v=u32();const auto identity=read.operator()<std::uint64_t>();std::array<std::uint8_t,32> expected;for(auto& v:expected)v=read.operator()<std::uint8_t>();std::array<std::vector<Event>,2> trace;for(auto& list:trace){const auto n=u32();for(unsigned j=0;j<n;++j){Event event;for(auto& v:event)v=u32();list.push_back(event);}}const auto counter_after=u32();
  parameter_dispatch::NotifierDirectory directory;parameter_dispatch::ParameterNotifier notifier(identity,directory);GuiIdTables tables;std::vector<std::uint32_t> keys;
  if(c[0]!=0xffffffffu)keys.push_back(c[0]);
  for(unsigned bank=0;bank<2;++bank){for(unsigned j=0;j<3;++j){const auto category=std::array<unsigned,3>{2,3,4}[j];if(c[9]&(1u<<j)){const auto id=(100+bank*1000+category)|(c[12]?0x80000000u:0);tables.features[bank][category]=std::bit_cast<std::int32_t>(id);keys.push_back(id);}}for(unsigned j=0;j<4;++j){const auto category=std::array<unsigned,4>{11,14,15,42}[j];if(c[10]&(1u<<j)){const auto id=(200+bank*1000+category)|(c[12]?0x80000000u:0);tables.properties[bank][category]=std::bit_cast<std::int32_t>(id);keys.push_back(id);}}}
  std::sort(keys.begin(),keys.end());keys.erase(std::unique(keys.begin(),keys.end()),keys.end());std::uint32_t counter=c[1];Context context;context.mode=c[2];GuiParameterDescriptor descriptor{c[4],c[6],c[5],c[7]};Backend backend;Observer observer;Notification notification;GuiNotifierRegistration concrete(notifier,directory,tables,counter,context);Trace registration(concrete,descriptor,c[11]);
  GuiRegisteredParameter owner(0x400000000ull,directory,backend,c[0],900,&observer,descriptor,notification,registration);
  bool bad=digest(owner,notifier,keys,counter,context,descriptor)!=expected||registration.events!=trace[0];events+=trace[0].size();registration.events.clear();context.mode=c[3];descriptor.flags204=c[8];owner.close();bad|=registration.events!=trace[1]||counter!=counter_after||owner.listener().size()!=0;events+=trace[1].size();for(auto key:keys)bad|=notifier.members(key,false)!=nullptr||notifier.members(key,true)!=nullptr;
  if(bad&&differences++<5)std::cerr<<"GUI registration mismatch "<<i<<'\n';
 }
 if(at!=bytes.size()||events!=2139)return 2;
 std::cout<<"{\"cases\":"<<cases<<",\"events\":"<<events<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
