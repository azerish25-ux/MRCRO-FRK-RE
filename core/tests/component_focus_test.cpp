#include "mfrk/ui_component_focus.hpp"
#include "mfrk/ui_knob_component.hpp"
#include "mfrk/ui_knob_visible_bounds.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
using Event=std::array<std::int32_t,12>;
struct Test;
struct Peer:GuiFocusPeer {
 Test* test;unsigned index;
 GuiDirtyBounds client_bounds()override;void invalidate(const GuiDirtyBounds&)override;
 bool minimized()override;void set_visible(bool)override;void grab_focus()override;bool focused()override;
};
struct Traverser:GuiKeyboardFocusTraverser {
 Test& test;unsigned index;
 Traverser(Test& t,unsigned i):test(t),index(i){}
 ~Traverser()override;
 GuiVisibilityComponent* default_component(GuiVisibilityComponent&)override;
};
struct Knob:GuiKnobComponentDisplayBoundary {
 using GuiKnobComponentDisplayBoundary::GuiKnobComponentDisplayBoundary;
 GuiKnobDisplayGeometry geometry()override{throw std::logic_error("Unexpected paint geometry lookup");}
};
struct Test:GuiComponentFocusBoundary {
 std::array<std::int32_t,17> config;
 std::array<GuiVisibilityComponent,3> nodes;
 std::array<Peer,3> peers{};
 std::array<bool,3> present{},minimized_state{},peer_focused{},accessible{},has_traverser{};
 std::array<std::int32_t,3> defaults{};
 std::vector<Event> events;
 GuiSliderPresentation* presentation=nullptr;
 Test(GuiComponentFocusState& state,const std::array<std::int32_t,17>& c):GuiComponentFocusBoundary(state),config(c){for(unsigned i=0;i<3;++i){peers[i].test=this;peers[i].index=i;}}
 unsigned index(const GuiRepaintComponent& c)const{for(unsigned i=0;i<3;++i)if(&nodes[i]==&c)return i;throw std::logic_error("Focus node identity");}
 std::int32_t focused_index()const{return focus_state.focused?static_cast<std::int32_t>(index(*focus_state.focused)):-1;}
 void set_focus(std::int32_t i){focus_state.focused=i<0?nullptr:&nodes[static_cast<unsigned>(i)];}
 void event(std::int32_t kind,unsigned i,std::initializer_list<std::int32_t> values={}){
  if(presentation&&presentation->live_component_flags()!=nodes[static_cast<unsigned>(config[1])].flags)throw std::logic_error("Actual component flags required");
  Event e{kind,static_cast<std::int32_t>(i)};std::copy(values.begin(),values.end(),e.begin()+2);events.push_back(e);
 }
 void callback(std::int32_t kind,unsigned i){
  if(config[10]!=kind||config[11]!=static_cast<std::int32_t>(i))return;
  if(config[12]==1)nodes[i].retire();
  else if(config[12]==2)set_focus(config[9]);
  else if(config[12]==3)nodes[i].parent=nullptr;
  else if(config[12]==4)nodes[i].focus_flags^=1u;
 }
 void focus_callback(std::int32_t kind,GuiVisibilityComponent& c,std::uint32_t reason){
  const auto i=index(c);const auto a=static_cast<std::int32_t>(focus_state.accessibility_focused),p=static_cast<std::int32_t>(focus_state.update_pending.load());
  if(kind==34)event(kind,i,{static_cast<std::int32_t>(reason),c.focus_flags,focused_index(),a,p});
  else event(kind,i,{static_cast<std::int32_t>(reason),focused_index(),a,p});
  callback(kind,i);
 }
 GuiDirtyBounds visible_bounds(GuiRepaintComponent& c)override{event(1,index(c));const std::array<double,4> f{};GuiKnobVisibleBounds b(f,nullptr);return b.get(c.bounds);}
 GuiFocusPeer* peer(GuiRepaintComponent& c)override{const auto i=index(c);event(3,i);return present[i]?&peers[i]:nullptr;}
 void fake_mouse_move(GuiVisibilityComponent& c)override{event(10,index(c));}
 void visibility_changed(GuiVisibilityComponent& c)override{event(14,index(c),{c.flags,c.requested_visible});}
 void hierarchy_changed(GuiVisibilityComponent& c)override{event(16,index(c),{c.flags,c.requested_visible});}
 void effective_visibility_changed(GuiVisibilityComponent& c,bool visible)override{event(20,index(c),{visible});}
 void emit_visibility(std::uintptr_t,bool)override{throw std::logic_error("Unexpected visibility signal");}
 void emit(std::uintptr_t)override{throw std::logic_error("Unexpected signal");}
 std::uintptr_t accessibility_handler(GuiVisibilityComponent& c)override{const auto i=index(c);event(38,i);return accessible[i]?i+1:0;}
 void notify_accessibility(std::uintptr_t handler,GuiAccessibilityEvent type)override{
  event(40,static_cast<unsigned>(handler),{static_cast<std::int32_t>(type),focused_index(),static_cast<std::int32_t>(focus_state.accessibility_focused),static_cast<std::int32_t>(focus_state.update_pending.load())});callback(40,static_cast<unsigned>(config[1]));
 }
 std::unique_ptr<GuiKeyboardFocusTraverser> keyboard_focus_traverser(GuiVisibilityComponent& c)override{
  const auto i=index(c);event(35,i);if(!has_traverser[i])return nullptr;return std::make_unique<Traverser>(*this,i);
 }
 void focus_gained(GuiVisibilityComponent& c,std::uint32_t reason)override{focus_callback(32,c,reason);}
 void focus_lost(GuiVisibilityComponent& c,std::uint32_t reason)override{focus_callback(33,c,reason);}
 void child_focus_changed(GuiVisibilityComponent& c,std::uint32_t reason)override{focus_callback(34,c,reason);}
 bool accessibility_is_ancestor(std::uintptr_t handler,std::uintptr_t focused)override{
  event(41,static_cast<unsigned>(handler),{static_cast<std::int32_t>(focused),focused_index(),static_cast<std::int32_t>(focus_state.accessibility_focused),static_cast<std::int32_t>(focus_state.update_pending.load())});return config[14]!=0;
 }
 void request_accessibility_focus(std::uintptr_t handler,bool parent)override{
  event(39,static_cast<unsigned>(handler),{parent,focused_index(),static_cast<std::int32_t>(focus_state.accessibility_focused),static_cast<std::int32_t>(focus_state.update_pending.load())});
  if(config[15])focus_state.accessibility_focused=handler;
  callback(39,static_cast<unsigned>(config[1]));
 }
 bool post_focus_update()override{
  event(29,0,{focused_index(),static_cast<std::int32_t>(focus_state.accessibility_focused),static_cast<std::int32_t>(focus_state.update_pending.load())});return config[7]!=0;
 }
 std::array<std::int32_t,19> snapshot(std::int32_t returned){
  std::array<std::int32_t,19> result{};
  for(unsigned i=0;i<3;++i){auto& c=nodes[i];result[i]=c.flags;result[3+i]=c.focus_flags;result[6+i]=c.requested_visible;result[9+i]=c.lifetime_guard()->component!=nullptr;result[12+i]=c.parent?static_cast<std::int32_t>(index(*c.parent)):-1;}
  result[15]=focused_index();result[16]=static_cast<std::int32_t>(focus_state.accessibility_focused);result[17]=static_cast<std::int32_t>(focus_state.update_pending.load());result[18]=returned;return result;
 }
};
GuiDirtyBounds Peer::client_bounds(){test->event(4,index);return {0,0,100,80};}
void Peer::invalidate(const GuiDirtyBounds& r){test->event(5,index,{r[0],r[1],r[2],r[3]});}
bool Peer::minimized(){test->event(25,index);return test->minimized_state[index];}
void Peer::set_visible(bool visible){test->event(24,index,{visible});}
void Peer::grab_focus(){
 test->event(30,index,{test->focused_index(),static_cast<std::int32_t>(test->focus_state.accessibility_focused),static_cast<std::int32_t>(test->focus_state.update_pending.load())});
 if(test->config[8]==1)test->set_focus(test->config[1]);
 else if(test->config[8]==2)test->set_focus(test->config[9]);
 else if(test->config[8]==3)test->nodes[static_cast<unsigned>(test->config[1])].retire();
}
bool Peer::focused(){test->event(31,index,{test->focused_index(),static_cast<std::int32_t>(test->focus_state.accessibility_focused),static_cast<std::int32_t>(test->focus_state.update_pending.load())});return test->peer_focused[index];}
Traverser::~Traverser(){test.event(37,index,{1});}
GuiVisibilityComponent* Traverser::default_component(GuiVisibilityComponent& c){test.event(36,index,{static_cast<std::int32_t>(test.index(c))});const auto i=test.defaults[index];return i<0?nullptr:&test.nodes[static_cast<unsigned>(i)];}
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;
 constexpr Identity expected={"component-focus","COMPF001",280620,768,1030917726u,{0x59,0x96,0xbf,0x5c,0x27,0x5,0x49,0xad,0xfa,0xea,0x57,0xe3,0xf1,0x6,0xa2,0xfb,0x7,0x41,0xf7,0xa5,0x43,0x97,0xe7,0x7e,0xed,0xff,0x99,0xfa,0x3a,0x58,0x2e,0x8a}};auto reader=Reader::file(argv[1],expected);unsigned differences=0,event_count=0;
 for(unsigned n=0;n<expected.count;++n){
  std::array<std::int32_t,17> config;for(auto& v:config)v=reader.i32();GuiComponentFocusState state;Test test(state,config);
  for(unsigned i=0;i<3;++i){std::array<std::int32_t,9> fields;for(auto& v:fields)v=reader.i32();auto& c=test.nodes[i];c.flags=static_cast<std::uint8_t>(fields[0]);c.focus_flags=static_cast<std::uint8_t>(fields[1]);c.bounds={0,0,100,80};c.parent=i<2&&fields[2]?&test.nodes[i+1]:nullptr;test.present[i]=fields[3];test.minimized_state[i]=fields[4];test.peer_focused[i]=fields[5];test.accessible[i]=fields[6];test.has_traverser[i]=fields[7];test.defaults[i]=fields[8];}
  for(unsigned i=0;i<2;++i)if(test.nodes[i].parent)test.nodes[i+1].children.push_back(&test.nodes[i]);
  test.set_focus(config[2]);state.accessibility_focused=static_cast<std::uintptr_t>(config[13]);state.update_pending=static_cast<std::uint32_t>(config[6]);
  std::array<std::int32_t,19> native;for(auto& v:native)v=reader.i32();const auto count=reader.u32();std::vector<Event> native_events(count);for(auto& e:native_events)for(auto& v:e)v=reader.i32();auto& target=test.nodes[static_cast<unsigned>(config[1])];const auto reason=static_cast<std::uint32_t>(config[3]);std::int32_t returned=-1;
  if(config[0]==0)test.grab_keyboard_focus(target,reason,config[4]!=0);
  else if(config[0]==1)gui_component_take_focus(target,reason,state,test);
  else if(config[0]==2)test.give_away_keyboard_focus(target,config[5]!=0);
  else if(config[0]==3)gui_component_focus_lost(target,reason,state,test);
  else if(config[0]==4){const auto guard=target.lifetime_guard();gui_component_child_focus_changed(target,reason,guard,state,test);}
  else if(config[0]==5)returned=gui_component_is_enabled(target);
  else if(config[0]==6){const auto* peer=gui_component_focus_peer(target,test);if(peer)for(unsigned i=0;i<3;++i)if(peer==&test.peers[i])returned=static_cast<std::int32_t>(i);}
  else {GuiSliderPresentation presentation;presentation.component_flags=target.flags^2u;{Knob knob(target,test,presentation);test.presentation=&presentation;knob.set_component_state(config[16]!=0);test.presentation=nullptr;}if(presentation.component_flags!=target.flags||presentation.component_flags_source)throw std::logic_error("Joined presentation teardown");}
  const auto actual=test.snapshot(returned);
  if(actual!=native||test.events!=native_events){if(differences++<5){std::cerr<<"Focus mismatch "<<n<<" entry="<<config[0]<<" state="<<(actual!=native)<<" events="<<test.events.size()<<'/'<<native_events.size()<<'\n';for(unsigned j=0;j<19;++j)if(actual[j]!=native[j])std::cerr<<" state "<<j<<' '<<actual[j]<<'/'<<native[j]<<'\n';for(unsigned j=0;j<std::min(test.events.size(),native_events.size());++j)if(test.events[j]!=native_events[j]){std::cerr<<" event "<<j<<" actual:";for(auto v:test.events[j])std::cerr<<' '<<v;std::cerr<<" expected:";for(auto v:native_events[j])std::cerr<<' '<<v;std::cerr<<'\n';break;}}}event_count+=count;
 }
 reader.end();if(event_count!=1750)return 2;std::cout<<"{\"cases\":"<<expected.count<<",\"events\":"<<event_count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
