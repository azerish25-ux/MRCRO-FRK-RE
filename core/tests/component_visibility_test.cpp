#include "mfrk/ui_knob_component.hpp"
#include "mfrk/ui_knob_visible_bounds.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
using Event=std::array<std::int32_t,12>;
struct Test;
struct Cache:GuiComponentImageCache {Test* test;unsigned index;bool invalidate_all()override;bool invalidate(const GuiDirtyBounds&)override;void release_resources()override;};
struct Peer:GuiVisibilityPeer {Test* test;unsigned index;GuiDirtyBounds client_bounds()override;void invalidate(const GuiDirtyBounds&)override;bool minimized()override;void set_visible(bool)override;};
struct Listener:GuiComponentVisibilityListener {Test* test;unsigned node,index;void visibility_changed(GuiVisibilityComponent&)override;void hierarchy_changed(GuiVisibilityComponent&)override;};
struct Hook:GuiVisibilityHook {Test* test;unsigned node,index;bool allow_visibility(GuiVisibilityComponent&,bool)override;};
struct WidgetListener:GuiWidgetVisibilityListener {Test* test;unsigned node,index;void changed()override;};
struct Knob:GuiKnobComponentDisplayBoundary {using GuiKnobComponentDisplayBoundary::GuiKnobComponentDisplayBoundary;GuiKnobDisplayGeometry geometry()override{throw std::logic_error("Visibility must not resolve knob paint geometry");}};
struct Test:GuiComponentVisibilityBoundary {
 std::array<std::int32_t,9> config;
 std::array<GuiVisibilityComponent,3> nodes;
 std::array<Cache,3> caches{};std::array<Peer,3> peers{};
 std::array<std::array<Listener,2>,3> listeners{};std::array<std::array<Hook,2>,3> hooks{};std::array<std::array<WidgetListener,2>,3> widgets{};
 std::array<std::array<float,6>,3> transforms{};std::array<GuiDirtyBounds,3> client{};
 std::array<bool,3> allowed{},present{},minimized_state{};std::array<std::array<bool,2>,3> hook_allow{};
 GuiRepaintComponent* focus=nullptr;std::vector<Event> events;GuiSliderPresentation* joined_presentation=nullptr;
 explicit Test(const std::array<std::int32_t,9>& c):config(c){for(unsigned i=0;i<3;++i){caches[i].test=this;caches[i].index=i;peers[i].test=this;peers[i].index=i;for(unsigned j=0;j<2;++j){listeners[i][j].test=this;listeners[i][j].node=i;listeners[i][j].index=j;hooks[i][j].test=this;hooks[i][j].node=i;hooks[i][j].index=j;widgets[i][j].test=this;widgets[i][j].node=i;widgets[i][j].index=j;}}}
 unsigned index(const GuiRepaintComponent& c)const{for(unsigned i=0;i<3;++i)if(&nodes[i]==&c)return i;throw std::logic_error("Visibility component identity");}
 void event(std::int32_t kind,unsigned i,std::initializer_list<std::int32_t> values={}){if(joined_presentation&&joined_presentation->live_component_flags()!=nodes[static_cast<unsigned>(config[1])].flags)throw std::logic_error("Presentation must borrow actual component flags");Event e{kind,static_cast<std::int32_t>(i)};std::copy(values.begin(),values.end(),e.begin()+2);events.push_back(e);}
 void mutate(unsigned i,bool own){if(i!=static_cast<unsigned>(config[1]))return;auto& list=nodes[i].component_listeners;if(config[6]==1&&own)list.resize(std::min(std::size_t(1),list.size()));if(config[6]==2&&!own)list.clear();if(config[6]==3&&own)list={&listeners[i][0],&listeners[i][1]};}
 void own(unsigned i,bool hierarchy){event(hierarchy?16:14,i,{nodes[i].flags,nodes[i].requested_visible});if(i==static_cast<unsigned>(config[1])&&config[5]==(hierarchy?3:1))nodes[i].retire();mutate(i,true);}
 void listener(unsigned i,unsigned slot,bool hierarchy){event(hierarchy?17:15,i,{static_cast<std::int32_t>(slot)});if(i==static_cast<unsigned>(config[1])&&config[5]==(hierarchy?4:2))nodes[i].retire();mutate(i,false);}
 GuiDirtyBounds visible_bounds(GuiRepaintComponent& c)override{const auto i=index(c);event(1,i);const std::array<double,4> fractions{};GuiKnobVisibleBounds bounds(fractions,nullptr);return bounds.get(c.bounds);}
 GuiVisibilityPeer* peer(GuiRepaintComponent& c)override{const auto i=index(c);event(3,i);return present[i]?&peers[i]:nullptr;}
 void fake_mouse_move(GuiVisibilityComponent& c)override{event(10,index(c));}
 GuiRepaintComponent* focused_component()override{return focus;}
 void grab_keyboard_focus(GuiRepaintComponent& c,std::uint32_t reason,bool parent)override{event(12,index(c),{static_cast<std::int32_t>(reason),parent});if(config[4]==1)focus=&c;else if(config[4]==2)focus=nullptr;}
 void give_away_keyboard_focus(GuiVisibilityComponent& c,bool notify)override{event(13,index(c),{notify});auto* p=focus;while(p&&p!=&c)p=p->parent;if(p)focus=nullptr;}
 void visibility_changed(GuiVisibilityComponent& c)override{own(index(c),false);}
 void hierarchy_changed(GuiVisibilityComponent& c)override{own(index(c),true);}
 std::uintptr_t hierarchy_peer_handle(GuiVisibilityComponent& c)override{const auto i=index(c);event(18,i);return present[i]?i+1:0;}
 void post_hierarchy_message(std::uintptr_t peer,std::uint32_t message)override{event(19,static_cast<unsigned>(peer),{static_cast<std::int32_t>(message)});}
 void effective_visibility_changed(GuiVisibilityComponent& c,bool visible)override{event(20,index(c),{visible});}
 void emit_visibility(std::uintptr_t signal,bool visible)override{event(22,static_cast<unsigned>(signal),{visible});if(config[8]){auto& c=nodes[(signal-100)/10];c.show_signal=0;c.hide_signal=0;}}
 void emit(std::uintptr_t signal)override{event(23,static_cast<unsigned>(signal));}
 std::array<std::int32_t,17> snapshot(std::int32_t showing){std::array<std::int32_t,17> result{};for(unsigned i=0;i<3;++i){const auto& c=nodes[i];result[i]=c.flags;result[3+i]=c.requested_visible;result[6+i]=nodes[i].lifetime_guard()->component!=nullptr;result[9+i]=static_cast<std::int32_t>(c.component_listeners.size());result[14+i]=(c.requested_signal?1:0)|(c.show_signal?2:0)|(c.hide_signal?4:0)|(c.shown_signal?8:0)|(c.hidden_signal?16:0);}result[12]=focus?static_cast<std::int32_t>(index(*focus)):-1;result[13]=showing;return result;}
};
bool Cache::invalidate_all(){test->event(2,index,{1});return test->allowed[index];}
bool Cache::invalidate(const GuiDirtyBounds& r){test->event(2,index,{0,r[0],r[1],r[2],r[3]});return test->allowed[index];}
void Cache::release_resources(){test->event(11,index);}
GuiDirtyBounds Peer::client_bounds(){test->event(4,index);return test->client[index];}
void Peer::invalidate(const GuiDirtyBounds& r){test->event(5,index,{r[0],r[1],r[2],r[3]});}
bool Peer::minimized(){test->event(25,index);return test->minimized_state[index];}
void Peer::set_visible(bool visible){test->event(24,index,{visible});if(index==static_cast<unsigned>(test->config[1])&&test->config[5]==5)test->nodes[index].retire();}
void Listener::visibility_changed(GuiVisibilityComponent&){test->listener(node,index,false);}
void Listener::hierarchy_changed(GuiVisibilityComponent&){test->listener(node,index,true);}
bool Hook::allow_visibility(GuiVisibilityComponent& c,bool visible){test->event(26,node,{static_cast<std::int32_t>(index),visible,c.flags,c.requested_visible});if(test->config[7]&&index==0)c.flags^=2;return test->hook_allow[node][index];}
void WidgetListener::changed(){test->event(21,node,{static_cast<std::int32_t>(index)});}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;constexpr Identity expected={"component-visibility","COMPVS01",301020,512,143569734u,{0x46,0x3d,0xdf,0x16,0x52,0x50,0xa4,0xf9,0x17,0x35,0xb2,0x63,0x62,0x4a,0x98,0xeb,0x14,0x32,0x21,0x75,0x4f,0xa9,0xbc,0x24,0x4f,0x3f,0x16,0xa4,0x87,0xf3,0x19,0xf0}};auto reader=Reader::file(argv[1],expected);unsigned differences=0,event_count=0;
 for(unsigned n=0;n<expected.count;++n){std::array<std::int32_t,9> config;for(auto& v:config)v=reader.i32();Test test(config);
  for(unsigned i=0;i<3;++i){std::array<std::int32_t,22> fields;for(auto& v:fields)v=reader.i32();auto& c=test.nodes[i];c.flags=static_cast<std::uint8_t>(fields[0]);c.parent=i<2&&fields[1]?&test.nodes[i+1]:nullptr;c.cached_image=fields[2]?&test.caches[i]:nullptr;test.allowed[i]=fields[3];test.present[i]=fields[4];c.requested_visible=fields[6];for(int j=0;j<fields[7];++j)c.component_listeners.push_back(&test.listeners[i][static_cast<unsigned>(j)]);for(int j=0;j<fields[8];++j)c.widget_listeners.push_back(&test.widgets[i][static_cast<unsigned>(j)]);for(int j=0;j<fields[9];++j)c.visibility_hooks.emplace(static_cast<unsigned>(j)+1,&test.hooks[i][static_cast<unsigned>(j)]);test.hook_allow[i]={fields[10]!=0,fields[11]!=0};const auto mask=fields[12];c.requested_signal=mask&1?100+i*10:0;c.show_signal=mask&2?101+i*10:0;c.hide_signal=mask&4?102+i*10:0;c.shown_signal=mask&8?103+i*10:0;c.hidden_signal=mask&16?104+i*10:0;test.minimized_state[i]=fields[13];std::copy_n(fields.begin()+14,4,c.bounds.begin());std::copy_n(fields.begin()+18,4,test.client[i].begin());for(auto& v:test.transforms[i])v=reader.f32();c.transform=fields[5]?&test.transforms[i]:nullptr;}
  for(unsigned i=0;i<2;++i)if(test.nodes[i].parent)test.nodes[i+1].children.push_back(&test.nodes[i]);
  if(config[3]>=0)test.focus=&test.nodes[static_cast<unsigned>(config[3])];
  std::array<std::int32_t,17> native;for(auto& v:native)v=reader.i32();const auto count=reader.u32();std::vector<Event> native_events(count);for(auto& e:native_events)for(auto& v:e)v=reader.i32();auto& target=test.nodes[static_cast<unsigned>(config[1])];std::int32_t showing=-1;
  if(config[0]==0){GuiSliderPresentation presentation;presentation.component_flags=target.flags^2u;{Knob knob(target,test,presentation);test.joined_presentation=&presentation;knob.set_component_state(config[2]!=0);test.joined_presentation=nullptr;}if(presentation.component_flags!=target.flags||presentation.component_flags_source)throw std::logic_error("Presentation flag detachment");}
  else if(config[0]==1)gui_component_set_visible(target,config[2]!=0,test);else if(config[0]==2)gui_component_visibility_changed(target,test);else if(config[0]==3)gui_component_hierarchy_changed(target,test);else if(config[0]==4)showing=gui_component_is_showing(target,test);else gui_component_release_image_resources(target);
  const auto state=test.snapshot(showing);if(state!=native||test.events!=native_events){if(differences++<5){std::cerr<<"Visibility mismatch "<<n<<" entry="<<config[0]<<" state="<<(state!=native)<<" events="<<test.events.size()<<'/'<<native_events.size()<<'\n';for(unsigned j=0;j<17;++j)if(state[j]!=native[j])std::cerr<<" state "<<j<<' '<<state[j]<<'/'<<native[j]<<'\n';for(unsigned j=0;j<std::min(test.events.size(),native_events.size());++j)if(test.events[j]!=native_events[j]){std::cerr<<" event "<<j<<" actual:";for(auto v:test.events[j])std::cerr<<' '<<v;std::cerr<<" expected:";for(auto v:native_events[j])std::cerr<<' '<<v;std::cerr<<'\n';break;}}}event_count+=count;
 }
 reader.end();if(event_count!=1535)return 2;std::cout<<"{\"cases\":"<<expected.count<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
