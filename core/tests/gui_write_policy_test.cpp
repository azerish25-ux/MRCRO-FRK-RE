#include "mfrk/ui_write_policy.hpp"
#include "mfrk/parameter_receiver.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
using namespace mfrk;using namespace mfrk::ui;
using Event=std::array<std::uint64_t,8>;
struct Test;
struct Transaction:GuiWriteTransaction {
 Test& test;explicit Transaction(Test& t):test(t){}bool busy()override;void prepare(std::uint32_t,std::uint32_t,std::uint32_t)override;
};
struct Test:GuiWritePolicyBoundary {
 std::array<std::uint32_t,27> input{};GuiWritePolicyState state;GuiWriteDescriptor desc,host_desc;std::vector<Event> events;Transaction transaction_owner{*this};GuiWriteReplacement replacements;
 void event(std::uint64_t kind,std::initializer_list<std::uint64_t> args={}){Event e{kind};std::copy(args.begin(),args.end(),e.begin()+1);events.push_back(e);}
 void begin(std::uint32_t id)override{event(3,{id});}
 void end(std::uint32_t id)override{event(4,{id});}
 void prewrite(std::uint32_t origin,std::uint32_t id,preset_bank::Value& value)override{event(1,{origin,id,value.scalar_bits()});}
 GuiWriteDescriptor& descriptor(std::uint32_t id)override{event(2,{id});return desc;}
 void special(std::uint32_t id,float value,std::uint32_t instrument)override{event(12,{id,std::bit_cast<std::uint32_t>(value),instrument});}
 GuiWriteTransaction* transaction()override{return input[15]?&transaction_owner:nullptr;}
 bool skip_transaction()override{event(19);return input[17]!=0;}
 std::vector<std::pair<std::uint32_t,float>> transforms(std::uint8_t part,std::uint32_t id,float value)override{event(7,{part,id,std::bit_cast<std::uint32_t>(value)});return {};}
 void backend_write(std::uint32_t origin,std::uint32_t id,preset_bank::Value& value)override{event(8,{origin,id,value.scalar_bits()});}
 void changed(std::uint32_t id,float value)override{event(29,{id,std::bit_cast<std::uint32_t>(value)});}
 void changed(preset_bank::Receiver& receiver,std::uint32_t message){event(30,{message,std::uint32_t(receiver.dirty()),receiver.value().scalar_bits(),receiver.cached_bits()});}
 bool feature_enabled(std::uint32_t category)override{event(9,{category});return input[19]!=0;}
 std::optional<std::uint32_t> feature(std::uint32_t category)override{event(26,{category});return input[0];}
 std::optional<std::uint32_t> reset_id(std::uint32_t instrument)override{event(27,{instrument});return 17;}
 void reset_note(std::uint32_t note)override{event(28,{note});}
 void association_single(std::uint32_t origin)override{event(20,{origin});}
 void association_many()override{event(21);}
 void association_parameter(std::uint32_t origin,std::uint32_t id)override{event(22,{origin,id});}
 std::optional<std::uint32_t> associated_id(const GuiWriteDescriptor&)override{event(23);return input[18]?std::optional<std::uint32_t>(17):std::nullopt;}
 void associated_value(std::uint32_t origin,std::uint32_t id,preset_bank::Value& value)override{event(24,{origin,id,value.scalar_bits()});}
 void association_changed(std::uint32_t id)override{event(25,{id});}
 std::optional<std::uint32_t> binding(unsigned group,unsigned index)override{event(14);return (input[23]==1&&group==0&&index==0)||(input[23]==2&&group==1&&index==16)?std::optional(input[0]):std::nullopt;}
 void save_part(std::uint32_t part,std::uint32_t id)override{event(13,{part,id});}
 bool associated_rows_empty(std::uint32_t id)override{event(11,{id});return input[20]!=0;}
 bool part_context_allowed(std::uint32_t)override{return !input[21];}
 const GuiWriteReplacement* replacement(std::uint32_t)override{return input[22]?&replacements:nullptr;}
 GuiWriteDescriptor& host_descriptor(std::uint32_t id)override{event(10,{id});return host_desc;}
 void host_edit(std::uint32_t id,double value)override{event(6,{id,std::bit_cast<std::uint64_t>(value)});}
 void processor(std::uint32_t id,preset_bank::Value& value)override{event(5,{id,value.tag(),value.scalar_bits()});}
 void notify(std::uint32_t id,preset_bank::Value& value,std::uint32_t origin){event(15,{id,value.scalar_bits(),origin});}
};
bool Transaction::busy(){test.event(17);return test.input[16]!=0;}
void Transaction::prepare(std::uint32_t id,std::uint32_t part,std::uint32_t group){test.event(18,{id,part,group});}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 std::ifstream f(argv[1],std::ios::binary|std::ios::ate);if(!f||f.tellg()!=1207756)return 2;
 std::vector<std::uint8_t> bytes(1207756);f.seekg(0);if(!f.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))return 2;
 if(ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x6b,0x8c,0x74,0x04,0xa6,0x62,0x89,0xbb,0x84,0x39,0xc2,0x45,0xbe,0x9d,0x34,0x7f,0x2f,0xf7,0x4b,0x9d,0xea,0x34,0x72,0xd5,0xed,0x04,0x8d,0x83,0x39,0x62,0x52,0x9a})return 2;
 f.seekg(0);std::array<char,8> magic{};f.read(magic.data(),8);if(std::string(magic.data(),8)!="GUIWP003")return 2;
 auto read=[&]<class T>(){T v;if(!f.read(reinterpret_cast<char*>(&v),sizeof(v)))throw std::runtime_error("Fixture bounds");return v;};auto word=[&](){return read.operator()<std::uint32_t>();};const auto cases=word();if(cases!=2160)return 2;unsigned differences=0,event_count=0;
 for(unsigned i=0;i<cases;++i){std::array<std::uint32_t,27> c;for(auto& v:c)v=word();std::array<std::uint32_t,6> expected;for(auto& v:expected)v=word();const auto expected_cache=read.operator()<std::uint64_t>();std::vector<Event> events(word());for(auto& e:events)for(auto& v:e)v=read.operator()<std::uint64_t>();
  Test test;test.input=c;test.state.mode11c30=c[3];test.state.enabledcca8=c[6];test.state.suppressedcca9=c[7];test.state.flage010=c[8];test.state.automationdad8=c[9];test.state.flage118=c[25];test.desc.flags204=c[4];test.desc.flagsd0=c[5];test.desc.flags20=c[12];test.desc.count48=c[13];test.desc.flag250=c[14];
  if(c[24]){test.host_desc.flags20=1;test.host_desc.count48=1;}
  if(c[22]){test.replacements.ids[c[22]-1]=c[0];test.replacements.scalar_bits={0x3e000000,0x3e800000,0x3f400000};}
  preset_bank::InternalReceiver raw_receiver{c[0],preset_bank::Value::scalar(0x3f000000),false};GuiInternalWriteReceiver<Test> receiver(raw_receiver,test);auto positive=preset_bank::Receiver::make(c[0],preset_bank::Value::scalar(0x3f000000),0x3fd0000000000000);GuiPositiveWriteReceiver<Test> positive_receiver(*positive.get(),test);GuiWriteReceiver* selected=c[26]?static_cast<GuiWriteReceiver*>(&positive_receiver):&receiver;
  auto incoming=preset_bank::Value::scalar(c[10]);const bool result=gui_apply_write(test.state,c[2],c[0],incoming,c[11]?selected:nullptr,c[1],test);
  const std::array<std::uint32_t,6> actual{std::uint32_t(result),selected->stored().scalar_bits(),incoming.scalar_bits(),c[26]?std::uint32_t(positive->dirty()):std::uint32_t(raw_receiver.dirty),test.state.automationdad8,test.state.performeddad4};
  if(actual!=expected||(c[26]?positive->cached_bits():0)!=expected_cache||test.events!=events){if(differences++<2){std::cerr<<"Policy mismatch "<<i<<" state="<<(actual!=expected)<<" events="<<(test.events!=events)<<" lengths="<<test.events.size()<<"/"<<events.size()<<'\n';for(unsigned j=0;j<std::min(test.events.size(),events.size());++j)if(test.events[j]!=events[j]){std::cerr<<" event "<<j<<" kinds "<<test.events[j][0]<<'/'<<events[j][0]<<'\n';break;}}}event_count+=events.size();
 }
 if(event_count!=14011||f.peek()!=std::char_traits<char>::eof())return 2;
 std::cout<<"{\"cases\":"<<cases<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
