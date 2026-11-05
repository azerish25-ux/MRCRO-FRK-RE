#include "mfrk/ui_scene_scope.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <cmath>
#include <bit>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
using namespace mfrk::ui::scene::scope;
using namespace mfrk::ui::scene::scope::test;

Node node(Reader& r,unsigned depth=0){if(depth>128)throw std::runtime_error("depth");Node n;n.type=1;n.name=r.blob(2*1024*1024);n.value=r.blob(2*1024*1024);auto a=r.u32();if(a>8192)throw std::runtime_error("attributes");for(unsigned i=0;i<a;++i){auto k=r.blob(2*1024*1024),v=r.blob(2*1024*1024);n.attributes.emplace_back(k,v);}auto c=r.u32();if(c>32768)throw std::runtime_error("children");for(unsigned i=0;i<c;++i)n.children.push_back(node(r,depth+1));return n;}
Context context(Reader& r){Context c;auto s=r.u32();if(s>4096)throw std::runtime_error("strings");for(unsigned i=0;i<s;++i){auto k=r.blob(2*1024*1024),v=r.blob(2*1024*1024);c.strings.emplace(k,v);}auto n=r.u32();if(n>4096)throw std::runtime_error("numbers");for(unsigned i=0;i<n;++i){auto k=r.blob(2*1024*1024);c.numbers.emplace(k,std::bit_cast<double>(r.u64()));}return c;}
bool same(const Context& a,const Context& b){if(a.strings!=b.strings||a.numbers.size()!=b.numbers.size())return false;for(const auto& [k,v]:a.numbers){auto p=b.numbers.find(k);if(p==b.numbers.end()||(std::bit_cast<std::uint64_t>(v)!=std::bit_cast<std::uint64_t>(p->second)&&!(std::isnan(v)&&std::isnan(p->second))))return false;}return true;}
bool same_node(const Node& a,const Node& b){if(a.name!=b.name||a.value!=b.value||a.attributes!=b.attributes||a.children.size()!=b.children.size())return false;for(std::size_t i=0;i<a.children.size();++i)if(!same_node(a.children[i],b.children[i]))return false;return true;}
struct Event {Node n;Context c;};
int main(int argc,char** argv){try{if(argc!=2)return 2;const auto& id=identity("scope");auto r=Reader::file(argv[1],id);const auto count=id.count;unsigned failures=0;
 for(unsigned index=0;index<count;++index){auto input=node(r);auto initial=context(r);std::map<unsigned,std::map<std::string,std::uint64_t>> plan;auto plans=r.u32();if(plans>64)return 2;for(unsigned i=0;i<plans;++i){auto event=r.u32(),changes=r.u32();if(changes>64)return 2;for(unsigned j=0;j<changes;++j){auto k=r.blob(2*1024*1024);plan[event].emplace(k,r.u64());}}
 auto event_count=r.u32();if(event_count>64)return 2;std::vector<Event> expected;for(unsigned i=0;i<event_count;++i){auto n=node(r);auto c=context(r);expected.push_back({std::move(n),std::move(c)});}auto after=context(r);auto current=initial;std::vector<Event> actual;bool failed=false;
 try{walk_nodes(input,current,[&](const Node& n,Context& c){auto i=unsigned(actual.size());actual.push_back({n,c});for(const auto& [key,bits]:plan[i])c.numbers.at(key)=std::bit_cast<double>(bits);});}catch(const std::exception& e){failed=true;std::cerr<<"case "<<index<<" exception "<<e.what()<<'\n';}
 if(!same(current,after)||actual.size()!=expected.size())failed=true;for(std::size_t i=0;i<actual.size()&&i<expected.size();++i)if(!same_node(actual[i].n,expected[i].n)||!same(actual[i].c,expected[i].c))failed=true;
 if(failed){++failures;std::cerr<<"DIFF "<<index<<'\n';}}
 r.end();std::cout<<count<<" "<<failures<<'\n';return failures?1:0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
