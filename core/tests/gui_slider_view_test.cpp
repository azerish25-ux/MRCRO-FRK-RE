#include "mfrk/ui_slider_view.hpp"
#include "mfrk/ui_bank_parameter.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <fstream>
#include <iostream>
using namespace mfrk;using namespace mfrk::ui;
using Event=std::array<std::uint32_t,12>;
struct Test;
struct Backend:GuiParameterBackend {
 Test& test;explicit Backend(Test& value):test(value){}
 GuiParameterValue& parameter(std::uint32_t)override;
 GuiParameterValue* mode_parameter(std::uint32_t,std::uint32_t)override{throw std::logic_error("View mode lookup");}
 void select(std::uint32_t,bool,bool,std::uintptr_t)override{throw std::logic_error("View selection");}
 GuiParameterOwner& owner()override{throw std::logic_error("View backend owner");}
 std::optional<std::uint32_t> selection(std::uint32_t,std::uint32_t)override{throw std::logic_error("View feature selector");}
 void begin(std::uint32_t,std::uint32_t)override{throw std::logic_error("View begin");}
 void end(std::uint32_t,std::uint32_t)override{throw std::logic_error("View end");}
 void write(std::uint32_t,std::uint32_t,preset_bank::Value&)override{throw std::logic_error("View write");}
 void changed(std::uint32_t,std::uintptr_t)override{throw std::logic_error("View backend changed");}
 void prepared(std::uint32_t,std::uintptr_t)override{throw std::logic_error("View backend prepared");}
 void ended(std::uint32_t,std::uintptr_t)override{throw std::logic_error("View backend ended");}
};
struct Parameter:GuiParameterValue {
 Test* test=nullptr;unsigned index=0;std::optional<GuiBankParameterView> view;
 const preset_bank::Value* current()override;
};
struct Test:GuiSliderViewBoundary,GuiControlFeatureBoundary {
 std::array<std::uint32_t,26> c;GuiSliderViewState view;GuiSliderParent parent{1,0,0},other_parent{2,0,0};GuiControlFeatureState state;GuiSliderViewObserver observer{view,*this};GuiControlParameters parameters{observer};GuiControlFeature feature{state,view.presentation,*this};Backend backend{*this};std::array<Parameter,2> parameter_views;std::array<GuiParameterDescriptor,2> descriptors;std::array<GuiParameterState,2> parameter_states{};std::array<std::unique_ptr<GuiParameterController>,2> controllers;preset_bank::Bank bank;std::vector<Event> events;
 explicit Test(const std::array<std::uint32_t,26>& input):c(input){
  for(unsigned i=0;i<2;++i){parameter_views[i].test=this;parameter_views[i].index=i;descriptors[i].typea8=i?2:1;parameter_states[i]={&backend,i?0x8000095au:13u,73,0,&observer,&descriptors[i],nullptr};controllers[i]=std::make_unique<GuiParameterController>(parameter_states[i]);observer.aliases.push_back({true,parameter_states[i].id,controllers[i].get()});}
  parameters.preferred_index=std::bit_cast<std::int32_t>(c[3]);view.selected_index=c[2];view.presentation.reverse=c[4];view.movable=c[5];view.orientation=std::bit_cast<std::int32_t>(c[6]);view.parent=c[7]?&parent:nullptr;view.signal=c[9]?1:0;view.presentation.scalar_cache=std::bit_cast<float>(c[10]);view.presentation.view_cache=std::bit_cast<float>(c[11]);view.presentation.feature_flags=c[12];view.presentation.component_flags=static_cast<std::uint8_t>(c[13]);state.reverse=c[16];state.flag09=c[17];state.mean_count=c[18];state.format=c[19];state.flag3c=c[20];state.active=c[21];state.value_index=std::bit_cast<std::int32_t>(c[22]);state.disabled_index=std::bit_cast<std::int32_t>(c[23]);state.disabled_data=c[24];
 }
 void setup(const std::vector<std::uint8_t>& bytes){
  if(!bank.positive.append(preset_bank::Receiver::make(13,preset_bank::Value::scalar(c[14]),0)))throw std::logic_error("View bank");
  for(unsigned i=0;i<=0x95a;++i)bank.internal.push_back({0x80000000u|i,preset_bank::Value::scalar(0),false});
  bank.internal.back().value=preset_bank::Value::data(bytes);
 }
 void event(std::uint32_t kind,std::initializer_list<std::uint32_t> values={}){Event e{kind};std::copy(values.begin(),values.end(),e.begin()+1);events.push_back(e);}
 std::uint32_t parameter_id(std::uint64_t index)override{return parameters.id(index);}
 float current(std::uint64_t index)override{return parameters.at(index).current();}
 float current(std::uint32_t id)override{return parameters.find(id).current();}
 bool is_data(std::uint32_t id)override{return parameters.find(id).is_data();}
 std::span<const std::uint8_t> data(std::uint32_t id)override{return parameters.find(id).current_data();}
 void scalar_changed(float previous,float current)override{event(3,{std::bit_cast<std::uint32_t>(previous),std::bit_cast<std::uint32_t>(current),std::bit_cast<std::uint32_t>(state.statistics[2])});}
 void range_changed(const std::array<float,5>& previous,const std::array<float,5>& current)override{Event e{4};for(unsigned i=0;i<5;++i){e[1+i]=std::bit_cast<std::uint32_t>(previous[i]);e[6+i]=std::bit_cast<std::uint32_t>(current[i]);}e[11]=std::bit_cast<std::uint32_t>(state.statistics[2]);events.push_back(e);}
 void set_component_state(bool active)override{event(10,{active});if(active)view.presentation.component_flags|=2u;else view.presentation.component_flags&=~2u;}
 void invalidate()override{
  event(5,{std::bit_cast<std::uint32_t>(view.presentation.view_cache),view.presentation.feature_flags,state.active,std::bit_cast<std::uint32_t>(state.statistics[0]),std::bit_cast<std::uint32_t>(state.statistics[1]),std::bit_cast<std::uint32_t>(state.statistics[2]),view.presentation.component_flags});
  if(c[25]&4u)if(view.signal)view.signal=2;
 }
 void set_bounds(const GuiSliderBounds& bounds)override{event(7,{static_cast<std::uint32_t>(bounds.x),static_cast<std::uint32_t>(bounds.y),static_cast<std::uint32_t>(bounds.width),static_cast<std::uint32_t>(bounds.height)});view.bounds=bounds;if(c[25]&2u){view.bounds.x+=7;view.bounds.y-=9;}if(c[25]&8u)view.parent=&other_parent;}
 bool smart_parent(std::uintptr_t identity)override{event(9,{static_cast<std::uint32_t>(identity)});return c[8]!=0;}
 void record_parent_bounds(std::uintptr_t identity,const GuiSliderBounds& bounds)override{event(8,{static_cast<std::uint32_t>(identity),static_cast<std::uint32_t>(bounds.x),static_cast<std::uint32_t>(bounds.y),static_cast<std::uint32_t>(bounds.width),static_cast<std::uint32_t>(bounds.height)});}
 void signal(std::uintptr_t identity,double value)override{const auto bits=std::bit_cast<std::uint64_t>(value);event(6,{static_cast<std::uint32_t>(identity),static_cast<std::uint32_t>(bits),static_cast<std::uint32_t>(bits>>32)});}
 void feature_changed(std::uint32_t id)override{feature.update(id);}
 std::array<std::uint32_t,17> snapshot(){
  std::array<std::uint32_t,17> out{std::bit_cast<std::uint32_t>(view.presentation.scalar_cache),std::bit_cast<std::uint32_t>(view.presentation.view_cache),view.presentation.feature_flags,view.presentation.component_flags,static_cast<std::uint32_t>(view.bounds.x),static_cast<std::uint32_t>(view.bounds.y),static_cast<std::uint32_t>(view.bounds.width),static_cast<std::uint32_t>(view.bounds.height),view.presentation.reverse,view.parent?static_cast<std::uint32_t>(view.parent->identity):0u,static_cast<std::uint32_t>(view.signal),state.active};for(unsigned i=0;i<5;++i)out[12+i]=std::bit_cast<std::uint32_t>(state.statistics[i]);return out;
 }
};
GuiParameterValue& Backend::parameter(std::uint32_t id){test.event(1,{id});const auto i=id==13?0u:1u;if(id!=13&&id!=0x8000095au)throw std::logic_error("View parameter ID");auto& view=test.parameter_views[i];view.view=GuiBankParameterView::resolve(test.bank,id);if(!view.view)throw std::logic_error("Actual view bank receiver");return view;}
const preset_bank::Value* Parameter::current(){test->event(2,{index});if(test->c[25]&1u)test->view.presentation.reverse=!test->view.presentation.reverse;return view->current();}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 std::ifstream f(argv[1],std::ios::binary|std::ios::ate);if(!f||f.tellg()!=589658)return 2;
 std::vector<std::uint8_t> bytes(589658);f.seekg(0);if(!f.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))return 2;
 if(ui::scene::scope::test::sha256(bytes)!=std::array<std::uint8_t,32>{0x1d,0xa2,0xb2,0xef,0x32,0x1b,0x0e,0x52,0x48,0x1b,0x00,0x62,0x95,0x0a,0x34,0xff,0xa5,0x7f,0x6d,0x7c,0x15,0xdf,0x66,0x77,0xe5,0x9f,0xfa,0xce,0x03,0x4d,0xaf,0x9b})return 2;
 f.seekg(0);std::array<char,8> magic;f.read(magic.data(),8);if(std::string(magic.data(),8)!="GUIVIEW1")return 2;
 auto read=[&]<class T>(){T v;if(!f.read(reinterpret_cast<char*>(&v),sizeof(v)))throw std::runtime_error("Fixture bounds");return v;};auto word=[&](){return read.operator()<std::uint32_t>();};const auto cases=word();if(cases!=928)return 2;unsigned differences=0,event_count=0;
 for(unsigned i=0;i<cases;++i){
  std::array<std::uint32_t,26> input;for(auto& v:input)v=word();Test test(input);test.state.slice_offset=read.operator()<std::uint64_t>();test.state.slice_length=read.operator()<std::uint64_t>();test.state.capacity=read.operator()<std::uint64_t>();test.state.disabled_offset=read.operator()<std::uint64_t>();
  test.view.bounds={std::bit_cast<std::int32_t>(word()),std::bit_cast<std::int32_t>(word()),std::bit_cast<std::int32_t>(word()),std::bit_cast<std::int32_t>(word())};test.parent.width=std::bit_cast<std::int32_t>(word());test.parent.height=std::bit_cast<std::int32_t>(word());test.other_parent.width=test.parent.width;test.other_parent.height=test.parent.height;
  for(auto& v:test.state.statistics)v=std::bit_cast<float>(word());
  const auto old_count=word();for(unsigned n=0;n<old_count;++n)test.state.history.push_back(std::bit_cast<float>(word()));const auto length=word();std::vector<std::uint8_t> packet(length);if(!f.read(reinterpret_cast<char*>(packet.data()),length))return 2;test.setup(packet);
  std::array<std::uint32_t,17> expected;for(auto& v:expected)v=word();std::vector<std::uint32_t> history(word());for(auto& v:history)v=word();std::vector<Event> events(word());for(auto& e:events)for(auto& v:e)v=word();
  if(input[0]==0)test.observer.changed(input[1]);else if(input[0]==1)test.observer.scalar_changed(input[1],std::bit_cast<float>(input[15]));else test.feature.update(input[1]);
  std::vector<std::uint32_t> actual_history;for(auto v:test.state.history)actual_history.push_back(std::bit_cast<std::uint32_t>(v));
  if(test.snapshot()!=expected||actual_history!=history||test.events!=events){if(differences++<4){std::cerr<<"View mismatch "<<i<<" state="<<(test.snapshot()!=expected)<<" history="<<(actual_history!=history)<<" events="<<(test.events!=events)<<'\n';for(unsigned n=0;n<17;++n)if(test.snapshot()[n]!=expected[n])std::cerr<<" state "<<n<<" actual="<<test.snapshot()[n]<<" expected="<<expected[n]<<'\n';for(unsigned j=0;j<std::min(test.events.size(),events.size());++j)if(test.events[j]!=events[j]){std::cerr<<" event "<<j<<" kind="<<test.events[j][0]<<'/'<<events[j][0]<<'\n';break;}}}event_count+=events.size();
 }
 if(event_count!=1397||f.peek()!=std::char_traits<char>::eof())return 2;
 std::cout<<"{\"cases\":"<<cases<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
