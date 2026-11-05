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
 }
 std::cout<<guards<<" scope guards and lazy-clause regression passed\n";return 0;
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
