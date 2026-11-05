#include "mfrk/ui_scene_structure.hpp"
#include <iostream>
#include <stdexcept>
using namespace mfrk::ui::scene::structure;
int main(){unsigned count=0;auto reject=[&](auto action){bool rejected=false;try{action();}catch(const std::invalid_argument&){rejected=true;}if(!rejected)throw std::runtime_error("missing structure admission guard");++count;};
 try{
 reject([]{parse_native_xml(std::string(2*1024*1024+1,' '));});
 reject([]{std::string text;for(unsigned i=0;i<129;++i)text+="<a>";for(unsigned i=0;i<129;++i)text+="</a>";parse_native_xml(text);});
 reject([]{std::string text;for(unsigned i=0;i<32768;++i)text+="<a/>";parse_native_xml(text);});
 reject([]{std::string text="<a";for(unsigned i=0;i<8193;++i)text+=" x=''";text+="/>";parse_native_xml(text);});
 reject([]{to_style_node(parse_native_xml("<a/>"));});
 reject([]{to_style_node(parse_native_xml("<a>text</a>").children.front());});
 reject([]{XmlNode n;n.type=1;n.value=std::string(2*1024*1024+1,'x');to_style_node(n);});
 reject([]{to_style_node(parse_native_xml("<a x='&#0;'/>").children.front());});
 reject([]{Styles styles;StyleNode n{"style","",{}, {}};define_style(styles,n,"missing");});
 reject([]{Styles styles;StyleNode n{"style","",{{"title","C:B"}}, {}};define_style(styles,n,"C:B");});
 reject([]{Styles styles;StyleNode n{"style","",{{"title","A"}}, {}};define_style(styles,n,"A");define_style(styles,n,"A");});
 reject([]{Styles styles;StyleNode n{"style",std::string(1,'\0'),{{"title","A"}}, {}};define_style(styles,n,"A");});
 reject([]{Styles styles;StyleNode n{"style","",{{"title","A"}}, {}};define_style(styles,n,std::string(4097,'A'));});
 reject([]{Styles styles;StyleNode n{"style",std::string(2*1024*1024+1,'x'),{{"title","A"}}, {}};define_style(styles,n,"A");});
 const auto doc=parse_native_xml("<style title='A' x='1' x='2'/><style title='B:A' x='3'/>");Styles styles;for(const auto& element:doc.children){auto node=to_style_node(element);define_style(styles,node,node.attributes.front().second);}if(styles.at("B").attributes!=std::vector<std::pair<std::string,std::string>>{{"x","3"},{"x","2"}})throw std::runtime_error("element/parser/style join");
 std::cout<<count<<" structure guards and element/style join passed\n";return 0;
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
