#include "mfrk/ui_scene_structure.hpp"
#include "ui_scene_structure_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui::scene::structure;
using namespace mfrk::ui::scene::structure::test;
namespace {
StyleNode decode(Reader& r,unsigned depth,std::size_t& count){
 if(depth>128||++count>32768)throw std::runtime_error("style fixture tree bound");
 StyleNode out;out.name=r.blob(4096);out.value=r.blob(4096);auto attributes=r.u32();if(attributes>8192)throw std::runtime_error("style fixture attributes");
 for(std::uint32_t i=0;i<attributes;++i){auto key=r.blob(4096);auto value=r.blob(4096);out.attributes.emplace_back(std::move(key),std::move(value));}
 auto children=r.u32();if(children>32768)throw std::runtime_error("style fixture children");for(std::uint32_t i=0;i<children;++i)out.children.push_back(decode(r,depth+1,count));return out;
}
void number(std::string& s,std::uint32_t n){for(unsigned i=0;i<4;++i)s.push_back(static_cast<char>(n>>(8*i)));}
void text(std::string& s,const std::string& value){number(s,static_cast<std::uint32_t>(value.size()));s+=value;}
void encode(std::string& s,const StyleNode& n){text(s,n.name);text(s,n.value);number(s,static_cast<std::uint32_t>(n.attributes.size()));for(const auto& [k,v]:n.attributes){text(s,k);text(s,v);}number(s,static_cast<std::uint32_t>(n.children.size()));for(const auto& c:n.children)encode(s,c);}
std::string outcome(const Styles& styles,const StyleNode& input){std::string s;number(s,static_cast<std::uint32_t>(styles.size()));for(const auto& [key,node]:styles){text(s,key);encode(s,node);}encode(s,input);return s;}
}
int main(int argc,char** argv){try{
 if(argc!=2)return 2;const auto& id=identity("style");auto r=Reader::file(argv[1],id);Styles styles;std::uint32_t previous=0;unsigned failures=0;
 for(std::uint32_t i=0;i<id.count;++i){auto sequence=r.u32();if(sequence>40||(i==0&&sequence!=0)||(i!=0&&sequence!=previous&&sequence!=previous+1))throw std::runtime_error("style fixture sequence");if(sequence!=previous)styles.clear();previous=sequence;const auto title=r.blob(4096);std::size_t count=0;auto input=decode(r,1,count);const auto expected=r.blob();define_style(styles,input,title);if(outcome(styles,input)!=expected){++failures;std::cerr<<"style case "<<i<<" differs\n";}}
 r.end();std::cout<<id.count<<" style cases, "<<failures<<" differences\n";return failures?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}}
