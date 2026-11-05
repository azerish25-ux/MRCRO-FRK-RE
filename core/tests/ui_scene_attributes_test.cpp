#include "mfrk/ui_scene_prefix.hpp"
#include "ui_scene_fixture_decode.hpp"
#include <iostream>
using namespace mfrk::ui::scene;using namespace mfrk::ui::scene::test;
struct Case{Attributes local,style;Constants numbers;StringConstants strings;bool strict=false,boolean=false,otherwise=false,present=false;FlexProperties property;std::string text,expected;std::uint32_t kind=0;std::int32_t missing=0,maximum=2147483647,integer=0;};
int main(int argc,char**argv)try{if(argc!=3)throw std::runtime_error("kind/file required");std::string kind=argv[1];auto&id=identity(kind);auto r=Reader::file(argv[2],id);std::vector<Case>cases;
for(unsigned i=0;i<id.count;++i){Case c;if(kind=="properties"){c.local=attrs(r);c.style=attrs(r);c.numbers=numbers(r);c.strings=strings(r);c.strict=r.flag();for(auto&x:c.property.values)x=r.i32();c.property.force_scale=r.flag();}
else if(kind=="numbers"){c.kind=r.u32();if(c.kind>1)throw std::runtime_error("number case kind");c.text=string(r);c.local=attrs(r);c.style=attrs(r);c.numbers=numbers(r);c.missing=r.i32();c.maximum=r.i32();if(c.kind==0){c.present=r.flag();c.expected=string(r);}else c.integer=r.i32();}
else if(kind=="conditions"){c.local=attrs(r);c.numbers=numbers(r);c.strings=strings(r);c.strict=r.flag();c.boolean=r.flag();c.otherwise=r.flag();}
else if(kind=="strings"){c.strict=r.flag();c.text=string(r);c.strings=strings(r);c.expected=string(r);}else {throw std::runtime_error("kind");}
cases.push_back(std::move(c));}r.end();unsigned bad=0;
for(const auto&c:cases){bool mismatch=false;try{if(kind=="properties"){auto a=read_flex_properties(c.local,c.style,c.numbers,c.strings,c.strict);mismatch=a.values!=c.property.values||a.force_scale!=c.property.force_scale;}else if(kind=="numbers"){if(c.kind==0){auto a=attribute_value(c.text,c.local,c.style);mismatch=a.has_value()!=c.present||(a&&*a!=c.expected);}else mismatch=read_integer_attribute(c.text,c.local,c.style,c.numbers,c.missing,c.maximum)!=c.integer;}else if(kind=="conditions"){bool a=select_if(c.local,c.numbers,c.strings,c.strict);mismatch=a!=c.boolean||!a!=c.otherwise;}else mismatch=expand_aliases(c.text,c.strings,c.strict)!=c.expected;}catch(const std::exception&){mismatch=true;}if(mismatch){if(bad<5)std::cerr<<"case "<<(&c-cases.data())<<" "<<c.text<<" mismatch\n";++bad;}}
std::cout<<kind<<": "<<cases.size()<<" original cases, "<<bad<<" mismatches\n";return bad?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
