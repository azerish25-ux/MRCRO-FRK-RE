#include "mfrk/ui_scene_structure.hpp"
#include <array>
#include <stdexcept>
namespace mfrk::ui::scene::structure {
namespace {
#include "ui_scene_xml_tables.inc"
class Parser {
 std::string_view s;std::size_t p=0,nodes=0;
 [[noreturn]] void fail(const char* message)const{throw std::invalid_argument(message);}
 unsigned char at(std::size_t i)const{return i<s.size()?static_cast<unsigned char>(s[i]):0;}
 unsigned char ch()const{return at(p);}
 bool starts(std::string_view value)const{return s.substr(p,value.size())==value;}
 void space(){while(whitespace_table[ch()])++p;}
 XmlNode node(std::uint32_t type){if(++nodes>32768)fail("XML adapter node bound");XmlNode out;out.type=type;return out;}
 std::string name(const std::array<unsigned char,256>& table){auto first=p;while(table[ch()])++p;return std::string(s.substr(first,p-first));}
 void until(std::string_view terminal){while(!starts(terminal)){if(!ch())fail("unexpected end of data");
   ++p;}p+=terminal.size();}
 void utf8(std::string& out,std::uint32_t value){
  if(value<0x80)out.push_back(char(value));
  else if(value<0x800){out.push_back(char((value>>6)|0xc0));out.push_back(char((value&63)|0x80));}
  else if(value<0x10000){out.push_back(char((value>>12)|0xe0));out.push_back(char(((value>>6)&63)|0x80));out.push_back(char((value&63)|0x80));}
  else if(value<0x110000){out.push_back(char((value>>18)|0xf0));out.push_back(char(((value>>12)&63)|0x80));out.push_back(char(((value>>6)&63)|0x80));out.push_back(char((value&63)|0x80));}
  else fail("invalid numeric character entity");
 }
 std::string decoded(const std::array<unsigned char,256>& table){
  std::string out;
  while(table[ch()]){
   if(ch()=='&'){
    if(at(p+1)=='#'){
     p+=2;std::uint32_t value=0,base=10;if(ch()=='x'){base=16;++p;}
     while(digit_table[ch()]!=255){value=value*base+digit_table[ch()];++p;}
     utf8(out,value);if(ch()!=';')fail("expected ;");++p;continue;
    }
    bool replaced=false;
    for(const auto& entry:std::array<std::pair<std::string_view,char>,5>{{{"&amp;",'&'},{"&apos;",'\''},{"&gt;",'>'},{"&lt;",'<'},{"&quot;",'"'}}})
     if(starts(entry.first)){out.push_back(entry.second);p+=entry.first.size();replaced=true;break;}
    if(replaced)continue;
   }
   out.push_back(char(ch()));++p;
  }
  return out;
 }
 XmlNode element(unsigned depth){
  if(depth>128)fail("XML adapter depth bound");
  auto out=node(1);out.name=name(name_table);if(out.name.empty())fail("expected element name");space();
  while(attribute_name_table[ch()]){
   auto key=name(attribute_name_table);space();if(ch()!='=')fail("expected =");++p;space();auto quote=ch();if(quote!='\''&&quote!='"')fail(R"(expected ' or ")");++p;
   auto value=decoded(quote=='\''?single_quote_table:double_quote_table);if(ch()!=quote)fail(R"(expected ' or ")");++p;space();
   if(out.attributes.size()>=8192)fail("XML adapter attribute bound");
   out.attributes.emplace_back(std::move(key),std::move(value));
  }
  if(ch()=='/') {++p;if(ch()!='>')fail("expected >");
  ++p;return out;}
  if(ch()!='>')fail("expected >");
  ++p;
  for(;;){
   const auto before_space=p;space();if(!ch())fail("unexpected end of data");
   if(ch()!='<'){
    p=before_space;auto data=node(2);data.value=decoded(text_table);
    if(out.value.empty()||out.value.front()==0)out.value=data.value;
    out.children.push_back(std::move(data));
   }
   if(ch()!='<')fail("unexpected end of data");
   ++p;
   if(ch()=='/') {++p;name(name_table);space();if(ch()!='>')fail("expected >");
  ++p;return out;}
   auto child=markup(depth+1);if(child.type!=0)out.children.push_back(std::move(child));
  }
 }
 XmlNode markup(unsigned depth){
  if(ch()=='?'){++p;until("?>");return {};}
  if(ch()!='!')return element(depth);
  ++p;
  if(starts("--")){p+=2;until("-->");return {};}
  if(starts("[CDATA[")){p+=7;auto out=node(3);const auto first=p;until("]]>");out.value=std::string(s.substr(first,p-first-3));return out;}
  if(starts("DOCTYPE")&&whitespace_table[at(p+7)]){
   p+=8;unsigned nesting=0;
   for(;;){if(!ch())fail("unexpected end of data");if(ch()=='>'&&nesting==0){++p;return {};}
    if(ch()=='[')++nesting;else if(ch()==']'&&nesting)--nesting;++p;}
  }
  until(">");return {};
 }
public:
 explicit Parser(std::string_view input):s(input){if(input.size()>2*1024*1024)fail("XML adapter input bound");s=s.substr(0,s.find('\0'));}
 XmlNode parse(){auto out=node(0);if(starts("\xef\xbb\xbf"))p+=3;for(;;){space();if(!ch())return out;if(ch()!='<')fail("expected <");++p;auto child=markup(1);if(child.type!=0)out.children.push_back(std::move(child));}}
};
}
XmlNode parse_native_xml(std::string_view input){return Parser(input).parse();}
}


#include <algorithm>
#include <stdexcept>
namespace mfrk::ui::scene::structure::detailstyle {
using Node=StyleNode;
namespace {
Node clone(const Node& in){
 Node out{in.name,{},in.attributes,{}};
 for(const auto& child:in.children)out.children.push_back(clone(child));
 return out;
}
void remove_first(Node& node,const std::string& key){
 auto& attrs=node.attributes;
 const auto p=std::find_if(attrs.begin(),attrs.end(),[&](const auto& a){return a.first==key;});
 if(p!=attrs.end())attrs.erase(p);
}
}
void define_style(Styles& styles,Node& input,const std::string& title){
 const auto colon=title.find(':');
 const std::string key=title.substr(0,colon);
 if(styles.contains(key))throw std::invalid_argument("duplicate style key");
 Node out;
 if(colon==std::string::npos)out=input;
 else{
  const auto base=styles.find(title.substr(colon+1));
  if(base==styles.end())throw std::invalid_argument("unknown base style");
  out=clone(base->second);
  for(const auto& attribute:input.attributes){
   remove_first(out,attribute.first);
   out.attributes.insert(out.attributes.begin(),attribute);
  }
  for(const auto& child:input.children){
   out.children.push_back(clone(child));
  }
  remove_first(out,"title");
  input.attributes.clear();
 }
 styles.emplace(key,std::move(out));
}
}


namespace mfrk::ui::scene::structure {
namespace {
void string_bound(const std::string& text,std::size_t& remaining){
 if(text.find('\0')!=std::string::npos||text.size()>remaining)throw std::invalid_argument("style string admission");
 remaining-=text.size();
}
void admit(const StyleNode& node,unsigned depth,std::size_t& count,std::size_t& bytes){
 if(depth>128||++count>32768||node.attributes.size()>8192)throw std::invalid_argument("style tree admission");
 string_bound(node.name,bytes);string_bound(node.value,bytes);
 for(const auto& [key,value]:node.attributes){string_bound(key,bytes);string_bound(value,bytes);}
 for(const auto& child:node.children)admit(child,depth+1,count,bytes);
}
StyleNode convert(const XmlNode& input,unsigned depth,std::size_t& count,std::size_t& bytes){
 if(input.type!=1||depth>128||++count>32768||input.attributes.size()>8192)throw std::invalid_argument("element-only style profile");
 string_bound(input.name,bytes);string_bound(input.value,bytes);
 for(const auto& [key,value]:input.attributes){string_bound(key,bytes);string_bound(value,bytes);}
 StyleNode out{input.name,input.value,input.attributes,{}};
 for(const auto& child:input.children)out.children.push_back(convert(child,depth+1,count,bytes));
 return out;
}
}
StyleNode to_style_node(const XmlNode& input){
 std::size_t count=0,bytes=2*1024*1024;return convert(input,1,count,bytes);
}
void define_style(Styles& styles,StyleNode& input,const std::string& title){
 std::size_t count=0,bytes=2*1024*1024;admit(input,1,count,bytes);
 if(title.size()>4096||title.find('\0')!=std::string::npos)throw std::invalid_argument("expanded style title admission");
 bool has_title=false;for(const auto& attribute:input.attributes)if(attribute.first=="title"){has_title=true;break;}
 if(!has_title)throw std::invalid_argument("missing source style title");
 const auto colon=title.find(':');const auto key=title.substr(0,colon);
 if(styles.contains(key))throw std::invalid_argument("duplicate style key");
 if(colon!=std::string::npos){const auto base=styles.find(title.substr(colon+1));if(base==styles.end())throw std::invalid_argument("unknown base style");count=0;bytes=2*1024*1024;admit(base->second,1,count,bytes);}
 detailstyle::define_style(styles,input,title);
}
}
