#include "mfrk/ui_scene_scope.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <unordered_set>
namespace mfrk::ui::scene::scope {
namespace {
using Constants=mfrk::ui::scene::Constants;
using Features=mfrk::ui::scene::Features;
using StringConstants=mfrk::ui::scene::StringConstants;
class LogicalExpression {
 std::string_view text;const Constants&constants;const Features*features;bool finite_only;std::size_t position=0;unsigned depth=0;
 void spaces(){while(position<text.size()&&text[position]==' ')++position;}
 char peek(){spaces();return position<text.size()?text[position]:0;}
 static bool digit(char c){return c>='0'&&c<='9';}
 double number(){spaces();double value=double(int(static_cast<unsigned char>(text[position++]))-48);
  while(digit(peek())){value=value*10.;value=value+double(static_cast<unsigned char>(text[position++]));value=value-48.;}
  if(peek()=='.'){++position;double weight=.1;while(digit(peek())){double term=double(int(text[position++])-48)*weight;weight=weight*.1;value=value+term;}}
  return value;
 }
 double name(){spaces();std::string_view key;
  if(position+1<text.size()&&text[position]=='$'&&text[position+1]=='{'){position+=2;auto end=text.find('}',position);if(end==std::string_view::npos)throw std::invalid_argument("unterminated constant");key=text.substr(position,end-position);position=end+1;}
  else{auto start=position;while(position<text.size()&&std::string_view(" ()*/\\%+-<>=!&|${").find(text[position])==std::string_view::npos)++position;key=text.substr(start,position-start);}
  if(key.empty()){throw std::invalid_argument("missing operand");}
  auto found=constants.find(std::string(key));if(found==constants.end()||(finite_only&&!std::isfinite(found->second)))throw std::invalid_argument("unknown or nonfinite constant");return found->second;
 }
 double primary(){if(++depth>32)throw std::invalid_argument("expression depth");char c=peek();double value;
  if(c=='!'){++position;value=source_zero(primary())?1.:0.;}
  else if(c=='+'||c=='-'){++position;value=primary();if(c=='-')value=-value;}
  else if(c=='('){++position;value=logical_or();if(peek()!=')')throw std::invalid_argument("parenthesis");++position;}
  else if(digit(c)||c=='.')value=number();
  else if(c==0)throw std::invalid_argument("missing operand");else {const auto open=text.find('(',position);const auto function=open==std::string_view::npos?std::string_view{}:text.substr(position,open-position);if(function=="count"||function=="feature"){const auto close=text.find(')',open+1);if(close==std::string_view::npos)throw std::invalid_argument("builtin closing parenthesis");const auto argument=text.substr(open+1,close-open-1);position=close+1;if(function=="count")value=argument.empty()?0.:double(1+std::count(argument.begin(),argument.end(),','));else{if(!features)throw std::invalid_argument("explicit feature registry required");value=features->contains(std::string(argument))?1.:0.;}}else value=name();}
  --depth;if(finite_only&&!std::isfinite(value))throw std::invalid_argument("nonfinite operand");return value;
 }
 static bool source_zero(double value){return std::isnan(value)||value==0.;}
 static double floor_quotient(double left,double right){double value=left/right;if(std::isfinite(value)&&value>=-9223372036854775808.&&value<9223372036854775808.){const auto integer=static_cast<std::int64_t>(value);if(double(integer)!=value)value=double(integer-(std::signbit(value)?1:0));}return value;}
 double product(){double value=primary();for(;;){char c=peek();if(c!='*'&&c!='/'&&c!='\\'&&c!='m'&&c!='M')break;++position;const double rhs=primary();if(c=='*')value=rhs*value;else if(c=='/')value=value/rhs;else if(c=='\\')value=floor_quotient(value,rhs);else if(c=='m')value=rhs<value?rhs:value;else value=value<rhs?rhs:value;if(finite_only&&!std::isfinite(value))throw std::invalid_argument("nonfinite arithmetic");}return value;}
 double sum(){double value=product();for(;;){char c=peek();if(c!='+'&&c!='-')break;++position;double rhs=product();value=c=='+'?value+rhs:value-rhs;if(finite_only&&!std::isfinite(value))throw std::invalid_argument("nonfinite arithmetic");}return value;}
 void consume(std::string_view token){for(char expected:token){if(peek()!=expected)throw std::invalid_argument("invalid native operator");++position;}}
 double relation(){double value=sum();for(;;){char c=peek();if(c!='<'&&c!='>'&&c!='l'&&c!='g')break;const bool less=c=='<'||c=='l';const bool named=c=='l'||c=='g';++position;bool equal=false;if(named){char tail=peek();if(tail!='e'&&tail!='t')throw std::invalid_argument("invalid relation");equal=tail=='e';++position;}else if(peek()=='='){equal=true;++position;}const double rhs=sum();value=(less?(equal?value<=rhs:value<rhs):(equal?value>=rhs:value>rhs))?1.:0.;}return value;}
 double equality(){double value=relation();for(;;){char c=peek();if(c!='!'&&c!='='&&c!='e'&&c!='n')break;const bool equal=c=='='||c=='e';consume(c=='!'?"!=":c=='='?"==":c=='e'?"eq":"ne");const double rhs=relation();const bool unordered=std::isnan(value)||std::isnan(rhs);value=(equal?(unordered||value==rhs):(!unordered&&value!=rhs))?1.:0.;}return value;}
 double logical_and(){double value=equality();for(;;){char c=peek();if(c!='&'&&c!='a')break;consume(c=='&'?"&&":"and");const double rhs=equality();value=(!source_zero(value)&&!source_zero(rhs))?1.:0.;}return value;}
 double logical_or(){double value=logical_and();for(;;){char c=peek();if(c!='|'&&c!='o')break;consume(c=='|'?"||":"or");const double rhs=logical_and();value=(!source_zero(value)||!source_zero(rhs))?1.:0.;}return value;}
public:
 LogicalExpression(std::string_view t,const Constants&c,const Features*f=nullptr,bool finite=true):text(t),constants(c),features(f),finite_only(finite){}
 double evaluate(){if(text.empty()||text.size()>1024)throw std::invalid_argument("expression length");double value=logical_or();if(peek()!=0||position!=text.size()||(finite_only&&!std::isfinite(value)))throw std::invalid_argument("unsupported expression or nonfinite result");return value;}
};

static std::int32_t condition_integer(double value){if(!std::isfinite(value)||value<-2147483648.||value>=2147483648.)return (-2147483647-1);return static_cast<std::int32_t>(value);}

static std::string expand_string(std::string_view input,const StringConstants&constants,bool strict,bool preserve_unknown=false){
 if(input.size()>1024||constants.size()>4096)throw std::invalid_argument("string environment bounds");
 for(const auto&[key,value]:constants)if(key.empty()||key.size()>1024||value.size()>4096)throw std::invalid_argument("string constant bounds");
 auto bounded=[](const std::string&value){if(value.size()>4096)throw std::invalid_argument("expanded string bounds");};
 if(!strict){auto found=constants.find(std::string(input));if(found!=constants.end())return found->second;std::string result(input);bool changed=false;for(const auto&[key,value]:constants){std::size_t position=0;for(;;){position=result.find(key,position);if(position==std::string::npos)break;result.replace(position,key.size(),value);position+=value.size();changed=true;bounded(result);}}if(changed){found=constants.find(result);if(found!=constants.end())return found->second;}return result;}
 std::size_t position=0;auto segment=[&](auto&&self,bool nested,unsigned depth)->std::string{if(depth>32)throw std::invalid_argument("placeholder depth");std::string result;while(position<input.size()){if(input[position]=='$'&&position+1<input.size()&&input[position+1]=='{'){position+=2;const auto key=self(self,true,depth+1);auto found=constants.find(key);if(found==constants.end()){if(!preserve_unknown)throw std::invalid_argument("unknown string constant");result+="${"+key+"}";}else result+=found->second;bounded(result);}else if(nested&&input[position]=='}'){++position;return result;}else{result+=input[position++];bounded(result);}}if(nested)throw std::invalid_argument("placeholder bracket mismatch");return result;};return segment(segment,false,0);
}
static bool literal_boolean(std::string_view text){if(text=="1"||text=="true")return true;if(text=="0"||text=="false")return false;throw std::invalid_argument("invalid boolean literal");}
using Attributes=mfrk::ui::scene::Attributes;
using mfrk::ui::scene::attribute_value;
bool select_if(const Attributes&local,const Constants&numbers,const StringConstants&strings,bool strict_strings,const Features*features){const auto value=attribute_value("constant",local,{});if(!value)throw std::invalid_argument("if requires constant");const auto text=expand_string(*value,strings,strict_strings,true);const double evaluated=LogicalExpression(text,numbers,features,false).evaluate();const auto integer=condition_integer(evaluated);const auto positive=attribute_value("positive_or_negative",local,{});return positive&&literal_boolean(*positive)?integer>0:integer==1;}
std::optional<std::string_view> field(const Node& node,const std::string& key){return mfrk::ui::scene::attribute_value(key,node.attributes,{});}
bool structural(std::string_view name){return name=="if"||name=="elseif"||name=="else"||name=="for"||name=="array"||name=="replace"||name=="group"||name.starts_with("template:");}
void walk(const Node& parent,Context& context,const Emit& emit,unsigned depth,unsigned flags){
 if(depth>128)throw std::invalid_argument("scope depth admission");
 bool chain=false,unmatched=false;
 for(const auto& child:parent.children){
  if(child.type!=1)throw std::invalid_argument("element sibling scope profile");
  if(child.name=="attribute")continue;
  if(child.name=="if"){
   const bool take=select_if(child.attributes,context.numbers,context.strings,true,context.features);
   if(take)walk(child,context,emit,depth+1,flags);
   unmatched=!take;chain=true;continue;
  }
  if(child.name=="elseif"){
   if(!chain)throw std::invalid_argument("elseif requires adjacent if/elseif");
   if(unmatched){const bool take=select_if(child.attributes,context.numbers,context.strings,true,context.features);if(take)walk(child,context,emit,depth+1,flags);unmatched=!take;}
   chain=true;continue;
  }
  if(child.name=="else"){
   if(!chain)throw std::invalid_argument("else requires adjacent if/elseif");
   if(unmatched)walk(child,context,emit,depth+1,flags);
   chain=false;continue;
  }
  chain=false;
  if(child.name=="with"){
   const auto bindings=collect_with(child,context);
   std::unordered_set<std::string> aliases,numbers;
   for(const auto& [key,value]:bindings.strings)if(context.strings.contains(key)||!aliases.insert(key).second)throw std::invalid_argument("scope string collision");
   for(const auto& [key,value]:bindings.numbers)if(context.numbers.contains(key)||context.strings.contains(key)||!numbers.insert(key).second||!aliases.insert(key).second)throw std::invalid_argument("scope number collision");
   for(const auto& [key,value]:bindings.strings)context.strings.emplace(key,value);
   for(const auto& [key,value]:bindings.numbers){context.numbers.emplace(key,value);context.strings.emplace(key,std::to_string(condition_integer(value)));}
   walk(child,context,emit,depth+1,flags);
   for(const auto& [key,value]:bindings.strings)context.strings.erase(key);
   for(const auto& [key,value]:bindings.numbers){context.numbers.erase(key);context.strings.erase(key);}
  }else if(child.name=="for"){
   const auto name=field(child,"src"),from=field(child,"from"),to=field(child,"to"),index=field(child,"index");
   if(!name||name->empty()||name->size()>1024||name->find('\0')!=std::string_view::npos||name->find("${")!=std::string_view::npos||!from||!to||field(child,"src3")||field(child,"in"))throw std::invalid_argument("for outside numeric range profile");
   auto number=[&](std::string_view value){auto expanded=expand_string(value,context.strings,true,true);const double v=LogicalExpression(expanded,context.numbers,context.features,false).evaluate();if(!std::isfinite(v)||v<double(std::numeric_limits<int>::min())||v>double(std::numeric_limits<int>::max()))throw std::invalid_argument("for numeric admission");return condition_integer(v);};
   const auto first=number(*from),last=number(*to);
   const auto inclusive=field(child,"inclusive");const bool include_end=!inclusive||literal_boolean(*inclusive);
   const auto distance=first<=last?std::int64_t(last)-first:std::int64_t(first)-last;
   if(distance>4095)throw std::invalid_argument("for iteration admission");
   const std::string key(*name),idx=index?std::string(*index):std::string();
   if((index&&(idx.empty()||idx.size()>1024||idx.find('\0')!=std::string::npos||idx.find("${")!=std::string::npos||idx==key))||context.strings.contains(key)||context.numbers.contains(key)||(index&&(context.strings.contains(idx)||context.numbers.contains(idx))))throw std::invalid_argument("for binding collision");
   const auto count=distance+(include_end?1:0);
   const auto name2=field(child,"src2"),from2=field(child,"from2"),to2=field(child,"to2");
   std::string key2;int first2=0,last2=0;
   if(name2){
    if(index||!from2||!to2||name2->empty()||name2->size()>1024||name2->find('\0')!=std::string_view::npos||name2->find("${")!=std::string_view::npos)throw std::invalid_argument("zipped for admission");
    key2=std::string(*name2);if(key2==key||context.strings.contains(key2)||context.numbers.contains(key2))throw std::invalid_argument("zipped for binding collision");
    first2=number(*from2);last2=number(*to2);
    const auto distance2=first2<=last2?std::int64_t(last2)-first2:std::int64_t(first2)-last2;
    if(distance2!=distance)throw std::invalid_argument("zipped for range lengths");
   }else if(from2||to2)throw std::invalid_argument("zipped for missing source");
   for(std::int64_t i=0;i<count;++i){
    const auto value=std::int64_t(first)+(first<=last?i:-i);
    context.strings[key]=std::to_string(value);if(flags&1)context.numbers[key]=double(value);
    if(name2){const auto value2=std::int64_t(first2)+(first2<=last2?i:-i);context.strings[key2]=std::to_string(value2);if(flags&1)context.numbers[key2]=double(value2);}
    if(index){context.strings[idx]=std::to_string(i+1);if(flags&1)context.numbers[idx]=double(i+1);}
    walk(child,context,emit,depth+1,flags);
   }
   context.strings.erase(key);if(flags&1)context.numbers.erase(key);
   if(name2){context.strings.erase(key2);if(flags&1)context.numbers.erase(key2);}
   if(index){
    if(include_end){context.strings.erase(idx);if(flags&1)context.numbers.erase(idx);}
    else{context.strings[idx]=std::to_string(count+1);if(flags&1)context.numbers[idx]=double(count+1);}
   }
  }else{
   if(structural(child.name))throw std::invalid_argument("structural construct outside with profile");
   emit(child,context);
  }
 }
}
}
Bindings collect_with(const Node& node,const Context& context){
 Bindings result;
 for(unsigned index=1;index<=4097;++index){
  const std::string suffix=index==1?"":std::to_string(index);
  const auto name=field(node,"src"+suffix);if(!name)return result;
  if(index>4096||name->empty()||name->size()>1024||name->find('\0')!=std::string_view::npos)throw std::invalid_argument("with binding admission");
  const auto string=field(node,"string"+suffix),number=field(node,"double"+suffix),color=field(node,"color"+suffix);
  if(unsigned(bool(string))+unsigned(bool(number))+unsigned(bool(color))!=1)throw std::invalid_argument("with requires one value kind");
  if(color)throw std::invalid_argument("with color outside profile");
  if(string){
   auto value=expand_string(*string,context.strings,true,false);
   if(value.starts_with("item_list(")&&value.ends_with(')'))throw std::invalid_argument("item_list provider outside profile");
   result.strings.emplace_back(*name,std::move(value));
  }else{
   auto text=expand_string(*number,context.strings,true,true);
   result.numbers.emplace_back(*name,LogicalExpression(text,context.numbers,context.features,false).evaluate());
  }
 }
 throw std::invalid_argument("with ordinal admission");
}
void walk_nodes(const Node& parent,Context& context,const Emit& emit,unsigned flags){if(!emit)throw std::invalid_argument("missing terminal receiver");walk(parent,context,emit,1,flags);}
}
