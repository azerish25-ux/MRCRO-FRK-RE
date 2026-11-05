#include "mfrk/ui_scene_scope.hpp"
#include <iostream>
#include <stdexcept>
using namespace mfrk::ui::scene::scope;
Node node(std::string name,std::vector<std::pair<std::string,std::string>> attributes={},std::vector<Node> children={}){Node n;n.type=1;n.name=std::move(name);n.attributes=std::move(attributes);n.children=std::move(children);return n;}
Node parent(Node child){return node("root",{},{std::move(child)});}
int main(){unsigned guards=0;auto reject=[&](auto action){bool failed=false;try{action();}catch(const std::invalid_argument&){failed=true;}if(!failed)throw std::runtime_error("missing scope guard");++guards;};
 try{
 auto noop=[](const Node&,Context&){};
 reject([&]{Context c;walk_nodes(node("root"),c,{});});
 reject([&]{Context c;collect_with(node("with",{{"src","N"}}),c);});
 reject([&]{Context c;collect_with(node("with",{{"src","N"},{"string","a"},{"double","1"}}),c);});
 reject([&]{Context c;collect_with(node("with",{{"src","N"},{"color","0"}}),c);});
 reject([&]{Context c;collect_with(node("with",{{"src","N"},{"string","item_list(Param)"}}),c);});
 reject([&]{Context c;collect_with(node("with",{{"src",""},{"string","a"}}),c);});
 reject([&]{Context c;collect_with(node("with",{{"src","N"},{"string","${MISSING}"}}),c);});
 reject([&]{Context c;collect_with(node("with",{{"src","N"},{"double","MISSING"}}),c);});
 reject([&]{Context c;c.strings["N"]="old";walk_nodes(parent(node("with",{{"src","N"},{"string","new"}})),c,noop);});
 reject([&]{Context c;c.numbers["N"]=.25;walk_nodes(parent(node("with",{{"src","N"},{"double","1"}})),c,noop);});
 reject([&]{Context c;walk_nodes(parent(node("with",{{"src","N"},{"string","a"},{"src2","N"},{"double2","1"}})),c,noop);});
 reject([&]{Context c;walk_nodes(parent(node("elseif",{{"constant","1"}})),c,noop);});
 reject([&]{Context c;walk_nodes(parent(node("else")),c,noop);});
 reject([&]{Context c;walk_nodes(parent(node("for")),c,noop);});
 reject([&]{Context c;walk_nodes(parent(node("for",{{"src","item"},{"in","item_list(P)"}})),c,noop);});
 reject([&]{Context c;walk_nodes(parent(node("for",{{"src","I"},{"from","0"},{"to","2"},{"src2","J"},{"from2","0"},{"to2","1"}})),c,noop);});
 reject([&]{Context c;walk_nodes(parent(node("template:Example")),c,noop);});
 reject([&]{Context c;auto n=node("data");n.type=2;walk_nodes(parent(n),c,noop);});
 reject([&]{Context c;for(unsigned i=0;i<4097;++i)c.strings["K"+std::to_string(i)]="value";collect_with(node("with",{{"src","N"},{"string","literal"}}),c);});
 reject([&]{Context c;collect_with(node("with",{{"src","N"},{"double",std::string(1025,'0')}}),c);});
 reject([&]{Context c;c.strings["BIG"]=std::string(1024,'x');collect_with(node("with",{{"src","N"},{"string","${BIG}${BIG}${BIG}${BIG}${BIG}"}}),c);});
 Context c;unsigned emitted=0;auto input=node("root",{}, {node("if",{{"constant","1"}},{node("chosen")}),node("attribute",{}, {node("ignored")}),node("elseif"),node("else",{}, {node("with",{{"src","BAD"}})})});walk_nodes(input,c,[&](const Node& n,Context&){if(n.name!="chosen")throw std::runtime_error("inactive clause visited");++emitted;});if(emitted!=1)throw std::runtime_error("lazy clause count");
 // The property prefix receives the live scoped context, including source-valid
 // contexts larger than the old 256-entry / 1024-byte adapter limits.
 Context joined;for(unsigned i=0;i<259;++i)joined.strings["K"+std::to_string(i)]="v";
 joined.strings["LONG"]=std::string(2048,'x');
 auto scoped=parent(node("with",{{"src","LOCAL"},{"string","${LONG}"}},{node("widget")}));
 walk_nodes(scoped,joined,[&](const Node&,Context& live){if(mfrk::ui::scene::expand_aliases("${LOCAL}",live.strings,true)!=std::string(2048,'x'))throw std::runtime_error("scope/property context join");});
 if(joined.strings.contains("LOCAL"))throw std::runtime_error("scope cleanup");
 {
  TemplateDefinition definition{{{"name",TemplateKind::string,"fallback"},{"label",TemplateKind::string,"${name}"},{"n",TemplateKind::number,"1/4"}},node("template",{}, {node("widget")})};
  Templates templates{{"Example",definition}};Context c;c.templates=&templates;unsigned count=0;
  auto instance=parent(node("template:Example",{{"name","Alpha"},{"name","Beta"}}));
  walk_nodes(instance,c,[&](const Node& n,Context& live){if(n.name!="widget"||live.strings.at("name")!="Beta"||live.strings.at("label")!="Beta"||live.numbers.at("n")!=.25)throw std::runtime_error("template bindings");++count;});
  if(count!=1||!c.strings.empty()||!c.numbers.empty())throw std::runtime_error("template cleanup");
 }
 {
  TemplateDefinition definition{{{"a",TemplateKind::number,"0"},{"b",TemplateKind::integer,"0"}},node("template",{}, {node("widget")})};
  Templates templates{{"numeric",definition}};Context c;c.templates=&templates;c.numbers["X"]=2.;
  walk_nodes(parent(node("template:numeric",{{"a","${X}+1"},{"b","1/4"}})),c,[&](const Node&,Context& live){if(live.numbers.at("a")!=3.||live.strings.contains("a")||live.numbers.at("b")!=0.||live.strings.at("b")!="0")throw std::runtime_error("template numeric namespaces");});
  if(c.numbers.size()!=1||c.numbers.at("X")!=2.||!c.strings.empty())throw std::runtime_error("template numeric cleanup");
 }
 {
  auto parsed=mfrk::ui::scene::structure::parse_native_xml("<template id='dual'><attribute name='name' type='string'/><attribute name='label' type='string' default='${name}'/><attribute name='amount' type='double' default='1/4'/><widget/></template>");
  Templates templates;register_template(templates,parsed.children.at(0));Context c;c.templates=&templates;unsigned count=0;
  walk_nodes(parent(node("template:dual",{{"name","Regular"}})),c,[&](const Node&,Context& live){if(live.strings.at("label")!="Regular"||live.numbers.at("amount")!=.25)throw std::runtime_error("registered template values");++count;});
  if(count!=1||!c.strings.empty()||!c.numbers.empty())throw std::runtime_error("registered template cleanup");
 }
 {
  auto parsed=mfrk::ui::scene::structure::parse_native_xml("<template id='dual'><attribute name='name' type='string'/><attribute name='param' type='string'/><panel><for src='item' in='item_list(${param})' index='ITEM_IDX'><with src='INV' double='${ITEM_IDX}==1'><ibutton text='${item}' invert='${INV}'/></with></for></panel></template>");
  Templates templates;register_template(templates,parsed.children.at(0));Context c;c.templates=&templates;c.item_list=[](std::string_view p){if(p!="Sustain_Polarity")throw std::runtime_error("wrong template parameter");return std::vector<std::string>{"Regular","Inverted"};};std::vector<std::string> seen;
  Emit factory;factory=[&](const Node& n,Context& live){if(n.name=="ibutton")seen.push_back(mfrk::ui::scene::expand_aliases(n.attributes.at(0).second,live.strings,true)+":"+mfrk::ui::scene::expand_aliases(n.attributes.at(1).second,live.strings,true));else walk_nodes(n,live,factory,1);};
  walk_nodes(parent(node("template:dual",{{"name","Sustain Polarity"},{"param","Sustain_Polarity"}})),c,factory,1);
  if(seen!=std::vector<std::string>{"Regular:1","Inverted:0"}||!c.strings.empty()||!c.numbers.empty())throw std::runtime_error("template/loop/factory composition");
 }
 for(unsigned flags: {0u,1u}){
  Context loop;loop.numbers["SEED"]=.125;std::vector<std::string> seen;
  auto range=parent(node("for",{{"src","I"},{"from","1+1"},{"to","0"},{"inclusive","false"},{"index","IDX"}},{node("widget")}));
  walk_nodes(range,loop,[&](const Node&,Context& live){seen.push_back(live.strings.at("I")+":"+live.strings.at("IDX"));if((flags&1)&&(live.numbers.at("I")!=double(3-seen.size())||live.numbers.at("IDX")!=double(seen.size())))throw std::runtime_error("numeric loop aliases");if(!(flags&1)&&(live.numbers.contains("I")||live.numbers.contains("IDX")))throw std::runtime_error("unexpected numeric loop alias");},flags);
  if(seen!=std::vector<std::string>{"2:1","1:2"}||loop.strings.contains("I")||loop.strings.at("IDX")!="3"||loop.numbers.size()!=1+(flags&1)||((flags&1)&&loop.numbers.at("IDX")!=3.))throw std::runtime_error("range output or exclusive index state");
  Context inclusive;seen.clear();auto forward=parent(node("for",{{"src","I"},{"from","-2"},{"to","1"},{"index","IDX"}},{node("widget")}));
  walk_nodes(forward,inclusive,[&](const Node&,Context& live){seen.push_back(live.strings.at("I"));},flags);
  if(seen!=std::vector<std::string>{"-2","-1","0","1"}||!inclusive.strings.empty()||!inclusive.numbers.empty())throw std::runtime_error("inclusive range output or cleanup");
  Context empty;seen.clear();auto equal=parent(node("for",{{"src","I"},{"from","2"},{"to","2"},{"inclusive","false"},{"index","IDX"}},{node("widget")}));
  walk_nodes(equal,empty,[&](const Node&,Context&){seen.push_back("unexpected");},flags);
  if(!seen.empty()||empty.strings.at("IDX")!="1"||((flags&1)&&empty.numbers.at("IDX")!=1.))throw std::runtime_error("empty exclusive range index");
  Context zipped;seen.clear();auto pair=parent(node("for",{{"src","I"},{"from","0"},{"to","2"},{"src2","J"},{"from2","4"},{"to2","6"}},{node("widget")}));
  walk_nodes(pair,zipped,[&](const Node&,Context& live){seen.push_back(live.strings.at("I")+":"+live.strings.at("J"));if((flags&1)&&(live.numbers.at("I")!=double(seen.size()-1)||live.numbers.at("J")!=double(seen.size()+3)))throw std::runtime_error("zipped numeric aliases");},flags);
  if(seen!=std::vector<std::string>{"0:4","1:5","2:6"}||!zipped.strings.empty()||!zipped.numbers.empty())throw std::runtime_error("zipped range output or cleanup");
  Context list;list.strings["param"]="Osc1_Type";list.item_list=[](std::string_view parameter){if(parameter!="Osc1_Type")throw std::runtime_error("wrong parameter");return std::vector<std::string>{"Alpha","one,two","2"};};seen.clear();
  auto list_loop=parent(node("for",{{"src","item"},{"in","item_list(${param})"},{"index","ITEM_IDX"}},{node("widget")}));
  walk_nodes(list_loop,list,[&](const Node&,Context& live){seen.push_back(live.strings.at("item")+":"+live.strings.at("ITEM_IDX"));if(live.numbers.contains("item")||((flags&1)&&live.numbers.at("ITEM_IDX")!=double(seen.size())))throw std::runtime_error("item-list numeric aliases");},flags);
  if(seen!=std::vector<std::string>{"Alpha:1","one:2","two:3","2:4"}||list.strings.size()!=1||!list.numbers.empty())throw std::runtime_error("item-list output or cleanup");
  Context dual;dual.strings["param"]="Sustain_Polarity";dual.item_list=[](std::string_view){return std::vector<std::string>{"Regular","Inverted"};};seen.clear();
  auto buttons=parent(node("for",{{"src","item"},{"in","item_list(${param})"},{"index","ITEM_IDX"}}, {node("with",{{"src","INV"},{"double","${ITEM_IDX}==1"}},{node("ibutton",{{"text","${item}"},{"invert","${INV}"}})})}));
  walk_nodes(buttons,dual,[&](const Node&,Context& live){seen.push_back(live.strings.at("item")+":"+live.strings.at("INV"));if(live.numbers.at("INV")!=double(seen.size()==1))throw std::runtime_error("dual button inversion");},flags);
  if(seen!=std::vector<std::string>{"Regular:1","Inverted:0"}||dual.strings.size()!=1||!dual.numbers.empty())throw std::runtime_error("side panel nested scope");
 }
 std::cout<<guards<<" scope guards and lazy-clause regression passed\n";return 0;
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
