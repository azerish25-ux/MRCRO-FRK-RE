#include "mfrk/ui_scene_prefix.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <stdexcept>
namespace mfrk::ui::scene {
using runtime::compile_geometry;
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
static double evaluate_logical_expression(std::string_view text,const Constants&constants,const Features*features=nullptr){return LogicalExpression(text,constants,features).evaluate();}
std::optional<std::string_view> attribute_value(std::string_view key,const Attributes&local,const Attributes&style){for(const auto&[name,value]:local)if(name==key)return value;for(const auto&[name,value]:style)if(name==key)return value;return std::nullopt;}
static std::string expand_string(std::string_view input,const StringConstants&constants,bool strict,bool preserve_unknown=false){
 if(input.size()>1024||constants.size()>4096)throw std::invalid_argument("string environment bounds");
 for(const auto&[key,value]:constants)if(key.empty()||key.size()>1024||value.size()>4096)throw std::invalid_argument("string constant bounds");
 auto bounded=[](const std::string&value){if(value.size()>4096)throw std::invalid_argument("expanded string bounds");};
 if(!strict){auto found=constants.find(std::string(input));if(found!=constants.end())return found->second;std::string result(input);bool changed=false;for(const auto&[key,value]:constants){std::size_t position=0;for(;;){position=result.find(key,position);if(position==std::string::npos)break;result.replace(position,key.size(),value);position+=value.size();changed=true;bounded(result);}}if(changed){found=constants.find(result);if(found!=constants.end())return found->second;}return result;}
 std::size_t position=0;auto segment=[&](auto&&self,bool nested,unsigned depth)->std::string{if(depth>32)throw std::invalid_argument("placeholder depth");std::string result;while(position<input.size()){if(input[position]=='$'&&position+1<input.size()&&input[position+1]=='{'){position+=2;const auto key=self(self,true,depth+1);auto found=constants.find(key);if(found==constants.end()){if(!preserve_unknown)throw std::invalid_argument("unknown string constant");result+="${"+key+"}";}else result+=found->second;bounded(result);}else if(nested&&input[position]=='}'){++position;return result;}else{result+=input[position++];bounded(result);}}if(nested)throw std::invalid_argument("placeholder bracket mismatch");return result;};return segment(segment,false,0);
}
const Attributes* resolve_style(const Attributes&local,const StyleRegistry&registry,const StringConstants&strings,bool strict){for(const auto&[key,value]:local)if(key=="predefined-style"){const auto name=expand_string(value,strings,strict);auto found=registry.find(name);if(found==registry.end())throw std::invalid_argument("unknown predefined style");return &found->second;}return nullptr;}
std::int32_t read_integer_attribute(std::string_view key,const Attributes&local,const Attributes&style,const Constants&numbers,std::int32_t missing,std::int32_t maximum,const Features*features){auto text=attribute_value(key,local,style);if(!text)return missing;const double value=evaluate_logical_expression(*text,numbers,features);if(!std::isfinite(value)||value<=-2147483649.||value>=2147483648.)throw std::invalid_argument("attribute integer conversion");const auto result=static_cast<std::int32_t>(value);if(result>maximum)throw std::invalid_argument("attribute above maximum");return result;}
static bool literal_boolean(std::string_view text){if(text=="1"||text=="true")return true;if(text=="0"||text=="false")return false;throw std::invalid_argument("invalid boolean literal");}
bool read_boolean_attribute(std::string_view text,const Constants&numbers){auto name=text;if(text.size()>=3&&text.substr(0,2)=="${"&&text.back()=='}')name=text.substr(2,text.size()-3);auto found=numbers.find(std::string(name));if(found==numbers.end())return literal_boolean(text);const double value=found->second;if(!std::isfinite(value))throw std::invalid_argument("nonfinite boolean constant");
 // Native SystemUCRT lround -> decimal integer -> strict0/1 parser. The only
 // accepted rounded results are0 and1; these exact half-boundary comparisons
 // express that decision without a host math call or arithmetic truthiness.
 if(value>-.5&&value<.5){return false;}
 if(value>=.5&&value<1.5){return true;}
 throw std::invalid_argument("rounded boolean is not0/1");}
FlexProperties read_flex_properties(const Attributes&local,const Attributes&style,const Constants&numbers,const StringConstants&strings,bool strict_strings,const Features*features){
 constexpr std::array<std::string_view,10>names{"flex-order","flex","min-width","max-width","min-height","max-height","flex-margin-left","flex-margin-right","flex-margin-top","flex-margin-bottom"};constexpr std::array<int,10>defaults{0,-1,0,-1,0,-1,0,0,0,0};FlexProperties result;for(unsigned j=0;j<10;++j)result.values[j+1]=read_integer_attribute(names[j],local,style,numbers,defaults[j],2147483647,features);if(auto value=attribute_value("force-scale-resize",local,style))result.force_scale=read_boolean_attribute(*value,numbers);
 if(auto value=attribute_value("align-self",local,style)){const auto expanded=expand_string(*value,strings,strict_strings);constexpr std::array<std::string_view,5>labels{"auto-align","flex-start","flex-end","center","stretch"};bool matched=false;for(unsigned j=0;j<labels.size();++j)if(expanded==labels[j]){result.values[0]=j;matched=true;break;}if(!matched)throw std::invalid_argument("unknown align-self");}return result;
}
static std::int32_t condition_integer(double value){if(!std::isfinite(value)||value<-2147483648.||value>=2147483648.)return (-2147483647-1);return static_cast<std::int32_t>(value);}
bool select_if(const Attributes&local,const Constants&numbers,const StringConstants&strings,bool strict_strings,const Features*features){const auto value=attribute_value("constant",local,{});if(!value)throw std::invalid_argument("if requires constant");const auto text=expand_string(*value,strings,strict_strings,true);const double evaluated=LogicalExpression(text,numbers,features,false).evaluate();const auto integer=condition_integer(evaluated);const auto positive=attribute_value("positive_or_negative",local,{});return positive&&literal_boolean(*positive)?integer>0:integer==1;}
flex::Item build_item(const ItemInput&a){
 if(a.direction<0||a.direction>3||a.dimension_mode<0||a.dimension_mode>3)throw std::invalid_argument("unsupported item dimension mode");
 auto finite=[](double v){return std::isfinite(v)&&std::abs(v)<=1000000.;};
 for(int d:a.parent_dimensions)if(d<0||d>1000000)throw std::invalid_argument("parent dimension domain");
 for(double d:a.relative)if(!finite(d))throw std::invalid_argument("relative dimension domain");
 for(double d:a.absolute)if(!finite(d))throw std::invalid_argument("absolute dimension domain");
 for(float v:{a.view_scale,a.context_d4,a.desktop_scale})if(!finite(v)||v<0)throw std::invalid_argument("item scale domain");
 auto integer=[](double value){if(!std::isfinite(value)||value<=-2147483649.||value>=2147483648.)throw std::invalid_argument("unrepresentable dimension");return static_cast<std::int32_t>(value);};
 const float view_scale=a.context_present?a.view_scale:1.f,context_d4=a.context_present?a.context_d4:1.f;
 const bool scale_absolute=(a.parent_force_scale&&a.child_force_scale)||a.dimension_mode==2;
 auto dimension=[&](int axis){
  if(a.full_dimension_override[axis]||!(std::abs(a.relative[axis]-1.)>=.0001))return a.parent_dimensions[axis];
  if(scale_absolute)return integer(double(view_scale)*a.absolute[axis]+.5);
  if(a.dimension_mode==3)return integer((double(context_d4)*a.absolute[axis]+1.)*.5);
  return integer(double(a.parent_dimensions[axis])*a.relative[axis]+.5);
 };
 flex::Item result;result.size={-1,-1};result.minimum={0,0};result.maximum={-1,-1};result.align=4;
 result.order=a.properties[1];const int growth=a.properties[2];
 if(growth<0){result.size[0]=float(dimension(0));result.size[1]=float(dimension(1));}
 else{result.grow=float(growth);if(a.direction<2)result.size[1]=float(dimension(1));else result.size[0]=float(dimension(0));}
 float scale=1.f;if(scale_absolute)scale=view_scale;else if(a.dimension_mode==3)scale=a.desktop_scale;
 auto property=[&](int index){const int value=a.properties[index];return float(value)*(value==-1?1.f:scale);};
 result.minimum={property(3),property(5)};result.maximum={property(4),property(6)};
 result.align=a.properties[0];result.margin={{{property(7),property(8)},{property(9),property(10)}}};return result;
}
DeclaredChild compile_declared_geometry(const Attributes&local,const Attributes&style,Constants&numbers,const DeclaredGeometryInput&input,const StringConstants&strings,bool strict,const Features*features){
 if(static_cast<unsigned>(input.parent_kind)>3)throw std::invalid_argument("unknown parent kind");
 const bool has_parent=input.parent_kind!=DeclaredParent::none,is_smart=input.parent_kind==DeclaredParent::smart||input.parent_kind==DeclaredParent::flex,is_flex=input.parent_kind==DeclaredParent::flex;
 if(has_parent)for(int d:input.parent_dimensions)if(d<=0||d>1000000)throw std::invalid_argument("declared parent dimensions");for(int d:input.intrinsic)if(d<0||d>1000000)throw std::invalid_argument("declared intrinsic dimensions");if(input.initial_mode<0||input.initial_mode>3)throw std::invalid_argument("declared mode");
 if(has_parent){numbers["PW"]=double(input.parent_dimensions[0]);numbers["PH"]=double(input.parent_dimensions[1]);}if(is_smart&&input.previous){const auto&p=*input.previous;for(int v:p)if(v<-1000000||v>1000000)throw std::invalid_argument("previous bounds");numbers["PXB"]=double(p[0]);numbers["PXE"]=double(p[0]+p[2]);numbers["PXC"]=double(p[0]+p[2]/2);numbers["PYB"]=double(p[1]);numbers["PYE"]=double(p[1]+p[3]);numbers["PYC"]=double(p[1]+p[3]/2);}
 DeclaredChild result;result.properties=is_flex?read_flex_properties(local,style,numbers,strings,strict,features):input.initial_properties;auto get=[&](std::string_view key){return attribute_value(key,local,style);};auto eval=[&](std::string_view text){return evaluate_logical_expression(text,numbers,features);};std::optional<double>x,y;double default_width=double(input.intrinsic[0]),default_height=double(input.intrinsic[1]);auto ox=get("offset-x"),oy=get("offset-y");if(ox||oy){x=numbers["PXB"];y=numbers["PYB"];if(ox)x=eval(*ox)+numbers["PXE"];if(oy)y=eval(*oy)+numbers["PYE"];default_width=numbers["PXE"]-numbers["PXB"];default_height=numbers["PYE"]-numbers["PYB"];}
 auto ax=get("x"),ay=get("y"),cx=get("center-x"),cy=get("center-y");const bool center_x=cx&&!ox&&!ax,center_y=cy&&!oy&&!ay;if(center_x)x=eval(*cx);if(center_y)y=eval(*cy);if(ax)x=eval(*ax);if(ay)y=eval(*ay);if(!x||!y)throw std::invalid_argument("missing declared position");
 auto available=[](double size,double position,bool center){if(!center)return size-position;const double far=size-position;const double chosen=position<=far?position:far;return chosen+chosen;};numbers["FILL"]=available(numbers["PW"],*x,center_x);auto aw=get("w"),ah=get("h");const bool scale_w=aw&&*aw=="scale",scale_h=ah&&*ah=="scale";double width=default_width,height=default_height;if(aw&&!scale_w)width=eval(*aw);numbers["FILL"]=available(numbers["PH"],*y,center_y);if(scale_w&&scale_h)throw std::invalid_argument("both dimensions scale");if(ah&&!scale_h)height=eval(*ah);if(scale_h){if(input.intrinsic[0]<=0||input.intrinsic[1]<=0)throw std::invalid_argument("scale without intrinsic");height=(double(input.intrinsic[1])*width)/double(input.intrinsic[0]);}if(scale_w){if(input.intrinsic[0]<=0||input.intrinsic[1]<=0)throw std::invalid_argument("scale without intrinsic");width=(double(input.intrinsic[0])*height)/double(input.intrinsic[1]);}if(!(width>0.)||!(height>0.)||!std::isfinite(width)||!std::isfinite(height))throw std::invalid_argument("nonpositive/nonfinite declared size");if(center_x)*x=*x+width*-.5;if(center_y)*y=*y+height*-.5;result.raw={*x,*y,width,height};
 result.check_bounds=input.initial_check_bounds;if(auto value=get("check-bounds"))result.check_bounds=literal_boolean(*value);result.mode=input.initial_mode;auto resize=get("is-resizing"),desktop=get("using-desktop-scale"),scale=get("using-scale");const bool using_scale=scale&&read_boolean_attribute(*scale,numbers);if(resize||desktop||using_scale){const bool using_desktop=desktop&&literal_boolean(*desktop);if(using_scale&&using_desktop)throw std::invalid_argument("conflicting scale modes");const bool resizing=resize&&eval(*resize)>0.;result.mode=using_scale?2:using_desktop?3:resizing?1:0;}
 if(has_parent&&input.context_check_bounds&&result.check_bounds){if(*x<0.)result.failure=BoundsFailure::negative_x;else if(*y<0.)result.failure=BoundsFailure::negative_y;else if(width+*x>numbers.at("PW"))result.failure=BoundsFailure::right;else if(height+*y>numbers.at("PH"))result.failure=BoundsFailure::bottom;if(result.failure!=BoundsFailure::none)return result;}
 std::array<int,4>bounds;for(unsigned j=0;j<4;++j){const double value=result.raw[j];if(!std::isfinite(value)||value<=-2147483649.||value>=2147483648.)throw std::invalid_argument("declared conversion range");bounds[j]=static_cast<std::int32_t>(value);}result.bounds=bounds;return result;
}
DeclaredChild compile_declared_node(const Attributes&local,const StyleRegistry&registry,Constants&numbers,const DeclaredGeometryInput&input,const StringConstants&strings,bool strict,const Features*features){const Attributes empty;auto style=resolve_style(local,registry,strings,strict);return compile_declared_geometry(local,style?*style:empty,numbers,input,strings,strict,features);}
GeometryPipelineResult compile_geometry_pipeline(const Attributes&local,const StyleRegistry&registry,Constants&numbers,const GeometryPipelineInput&input,const StringConstants&strings,bool strict,const Features*features){GeometryPipelineResult result;result.declaration=compile_declared_node(local,registry,numbers,input.declaration,strings,strict,features);if(!result.declaration.bounds)return result;const auto kind=input.declaration.parent_kind;if(kind==DeclaredParent::smart||kind==DeclaredParent::flex)result.spec=compile_geometry(input.declaration.parent_dimensions,*result.declaration.bounds);if(kind==DeclaredParent::flex){ItemInput a;a.parent_dimensions=input.physical_parent;a.context_present=input.context_present;a.direction=input.direction;a.parent_force_scale=input.parent_force;a.child_force_scale=result.declaration.properties.force_scale;a.full_dimension_override=input.full_override;a.dimension_mode=result.declaration.mode;a.view_scale=input.view_scale;a.context_d4=input.context_d4;a.desktop_scale=input.desktop_scale;a.relative={result.spec->relative[2],result.spec->relative[3]};a.absolute={result.spec->absolute[2],result.spec->absolute[3]};a.properties=result.declaration.properties.values;result.item=build_item(a);}return result;}
double evaluate_expression(std::string_view text,const Constants&numbers,const Features*features){return evaluate_logical_expression(text,numbers,features);}
std::string expand_aliases(std::string_view text,const StringConstants&strings,bool strict){return expand_string(text,strings,strict,false);}
std::string expand_condition_aliases(std::string_view text,const StringConstants&strings,bool strict){return expand_string(text,strings,strict,true);}
bool parse_boolean_literal(std::string_view text){return literal_boolean(text);}
} // namespace mfrk::ui::scene
