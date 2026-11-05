#include "mfrk/ui_scene_structure.hpp"
#include "ui_scene_structure_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui::scene::structure;
using namespace mfrk::ui::scene::structure::test;
namespace {
void number(std::string& s,std::uint32_t n){for(unsigned i=0;i<4;++i)s.push_back(static_cast<char>(n>>(8*i)));}
void text(std::string& s,const std::string& value){number(s,static_cast<std::uint32_t>(value.size()));s+=value;}
void encode(std::string& s,const XmlNode& n){number(s,n.type);text(s,n.name);text(s,n.value);number(s,static_cast<std::uint32_t>(n.attributes.size()));for(const auto& [k,v]:n.attributes){text(s,k);text(s,v);}number(s,static_cast<std::uint32_t>(n.children.size()));for(const auto& c:n.children)encode(s,c);}
}
int main(int argc,char** argv){try{
 if(argc!=2)return 2;
 const auto& id=identity("xml");auto r=Reader::file(argv[1],id);unsigned failures=0;
 for(std::uint32_t i=0;i<id.count;++i){
  const auto input=r.blob(2*1024*1024);const auto expected_status=r.u32();if(expected_status>1)throw std::runtime_error("XML fixture status");const auto expected=r.blob();std::uint32_t status=0;std::string result;
  try{encode(result,parse_native_xml(input));}catch(const std::invalid_argument& error){status=1;result=error.what();}
  if(status!=expected_status||result!=expected){++failures;std::cerr<<"XML case "<<i<<" differs\n";}
 }
 r.end();std::cout<<id.count<<" XML cases, "<<failures<<" differences\n";return failures?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}}
