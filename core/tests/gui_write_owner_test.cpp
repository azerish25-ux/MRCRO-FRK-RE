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
#include "mfrk/ui_write_owner.hpp"
using OuterEvent=std::array<std::uint64_t,12>;
using Logical=std::array<std::uint64_t,6>;
Logical logical(const preset_bank::Value& value){
 Logical out{value.tag(),value.bytes().size()};const auto hash=ui::scene::scope::test::sha256(value.bytes());std::memcpy(out.data()+2,hash.data(),32);return out;
}
struct OuterTest:Test,GuiWriteLock,GuiHardwareWriteBoundary,GuiWriteDispatchBoundary {
 std::array<std::uint32_t,39> c;GuiHardwareWriteState hardware_state;bool held=false,recursing=false;std::vector<std::unique_ptr<GuiWriteMessage>> retained;std::vector<OuterEvent> outer_events;std::unique_ptr<GuiWriteMessage> queued;
 preset_bank::InternalReceiver internal; preset_bank::Ref positive;GuiInternalWriteReceiver<Test> internal_view;GuiPositiveWriteReceiver<Test> positive_view;GuiWriteReceiver* selected;
 explicit OuterTest(const std::array<std::uint32_t,39>& data):c(data),hardware_state{c[30]?c[0]:0xffffffffu,c[27],-37,c[28]!=0,c[29]!=0},internal{c[0],preset_bank::Value::scalar(0x3f000000),false},positive(preset_bank::Receiver::make(c[0],preset_bank::Value::scalar(0x3f000000),0x3fd0000000000000)),internal_view(internal,*this),positive_view(*positive.get(),*this),selected(c[26]?static_cast<GuiWriteReceiver*>(&positive_view):&internal_view){
  std::copy_n(c.begin(),27,input.begin());state.mode11c30=c[3];state.enabledcca8=c[6];state.suppressedcca9=c[7];state.flage010=c[8];state.automationdad8=c[9];state.flage118=c[25];desc.flags204=c[4];desc.flagsd0=c[5];desc.flags20=c[12];desc.count48=c[13];desc.flag250=c[14];
 }
 void outer_event(std::uint64_t kind,std::uint64_t a=0,std::uint64_t b=0,const preset_bank::Value* value=nullptr){
  OuterEvent event{kind,a,b};if(value){const auto v=logical(*value);std::copy(v.begin(),v.end(),event.begin()+3);}event[9]=hardware_state.guard;event[10]=held;outer_events.push_back(event);
 }
 void nested_write(){recursing=true;auto value=preset_bank::Value::scalar(0x3e000000);gui_write_owned(hardware_state,*this,*this,*this,2,c[0],value);recursing=false;}
 bool try_lock()override{outer_event(1,c[31]);held=!c[31];return held;}
 void unlock()override{if(!held)throw std::logic_error("Unlock without lock");outer_event(2);held=false;}
 bool eligible(std::uint32_t origin,std::uint32_t id,preset_bank::Value& value)override{
  if(held)throw std::logic_error("Eligibility under mutex");
  outer_event(3,origin,id,&value);if(c[37]&&!recursing)nested_write();if(c[33])value=preset_bank::Value::scalar(0x3e000000);return c[32]!=0;
 }
 void send_original(std::uint32_t id,preset_bank::Value& value)override{outer_event(6,0,id,&value);if(c[38]&&!recursing)nested_write();}
 std::optional<std::uint64_t> message_thread()override{return c[35]?std::optional<std::uint64_t>(111):std::nullopt;}
 std::uint32_t current_thread()override{outer_event(4);return 222;}
 void post(std::unique_ptr<GuiWriteMessage> message)override{outer_event(5,message->origin,message->id,&message->value);if(queued)retained.push_back(std::move(queued));queued=std::move(message);if(c[34])hardware_state.guard=false;}
 std::uint32_t part_count()override{return 3;}
 void prepare_part(std::uint32_t part)override{outer_event(7,part,0);}
 std::uintptr_t receiver(std::uint32_t id,std::uint32_t part)override{outer_event((id&0x80000000u)?8:9,id-part*1000000u);return 1;}
 bool apply(std::uint32_t origin,std::uint32_t id,preset_bank::Value& value,std::uintptr_t token,std::uint32_t part)override{if(token!=1)throw std::logic_error("Actual receiver token");return gui_apply_write(state,origin,id,value,selected,part,*this);}
 GuiWriteDescriptor& descriptor(std::uint32_t id)override{auto& result=Test::descriptor(id);if(c[34])hardware_state.guard=false;return result;}
};
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 std::ifstream file(argv[1],std::ios::binary|std::ios::ate);if(!file||file.tellg()!=336164)return 2;
 std::vector<std::uint8_t> bytes(336164);file.seekg(0);if(!file.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))return 2;
 if(ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0xe4,0x7a,0xb0,0xa4,0x82,0x0d,0x98,0x88,0xad,0x8e,0x79,0x3d,0x8c,0xb3,0x25,0xcf,0xc7,0xc1,0x0e,0xce,0xe0,0xeb,0x52,0x6d,0x9a,0x4d,0x75,0xc0,0x6a,0x3d,0x06,0x55})return 2;
 file.seekg(0);std::array<char,8> magic{};file.read(magic.data(),8);if(std::string(magic.data(),8)!="GUIWO002")return 2;
 auto read=[&]<class T>(){T result;if(!file.read(reinterpret_cast<char*>(&result),sizeof(result)))throw std::runtime_error("Fixture bounds");return result;};auto word=[&](){return read.operator()<std::uint32_t>();};const auto cases=word();if(cases!=474)return 2;unsigned differences=0,core_events=0,outer_events=0;
 for(unsigned i=0;i<cases;++i){
  std::array<std::uint32_t,39> c;for(auto& v:c)v=word();std::array<std::uint32_t,5> expected;for(auto& v:expected)v=word();const auto cache=read.operator()<std::uint64_t>();const auto cached=word(),guard=word();Logical input_logical;for(auto& v:input_logical)v=read.operator()<std::uint64_t>();const bool has_queued=word()!=0;Logical expected_queued{};if(has_queued)for(auto& v:expected_queued)v=read.operator()<std::uint64_t>();
  std::vector<Event> core_trace(word());for(auto& e:core_trace)for(auto& v:e)v=read.operator()<std::uint64_t>();std::vector<OuterEvent> outer_trace(word());for(auto& e:outer_trace)for(auto& v:e)v=read.operator()<std::uint64_t>();OuterTest test(c);
  auto value=preset_bank::Value::scalar(c[10]);if(c[36]){std::vector<std::uint8_t> raw(c[36]);for(unsigned j=0;j<raw.size();++j)raw[j]=static_cast<std::uint8_t>(37+j*17);value=preset_bank::Value::data(raw);}
  gui_write_owned(test.hardware_state,test,test,test,c[2],c[0],value);
  const std::array<std::uint32_t,5> actual{test.selected->stored().scalar_bits(),value.scalar_bits(),c[26]?std::uint32_t(test.positive->dirty()):std::uint32_t(test.internal.dirty),test.state.automationdad8,test.state.performeddad4};
  bool bad=actual!=expected||logical(value)!=input_logical||test.held||(c[26]?test.positive->cached_bits():0)!=cache||std::bit_cast<std::uint32_t>(test.hardware_state.cached_integer)!=cached||test.hardware_state.guard!=(guard!=0)||bool(test.queued)!=has_queued||test.events!=core_trace||test.outer_events!=outer_trace;
  if(test.queued)bad|=logical(test.queued->value)!=expected_queued;
  if(bad&&differences++<3){std::cerr<<"Owner mismatch "<<i<<" state="<<(actual!=expected)<<" core="<<(test.events!=core_trace)<<" outer="<<(test.outer_events!=outer_trace)<<'\n';for(unsigned j=0;j<std::min(test.outer_events.size(),outer_trace.size());++j)if(test.outer_events[j]!=outer_trace[j]){std::cerr<<" first outer event "<<j<<" kind "<<test.outer_events[j][0]<<'/'<<outer_trace[j][0]<<'\n';break;}}
  core_events+=core_trace.size();outer_events+=outer_trace.size();
 }
 if(core_events!=806||outer_events!=1627||file.peek()!=std::char_traits<char>::eof())return 2;
 std::cout<<"{\"cases\":"<<cases<<",\"policy_events\":"<<core_events<<",\"outer_events\":"<<outer_events<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
