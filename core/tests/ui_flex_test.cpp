#include "mfrk/ui_flex.hpp"
#include "mfrk/ui.hpp"
#include "ui_flex_fixture_cases.hpp"
#include <cfenv>
#include <iostream>
namespace {
using namespace mfrk::ui::flex;using namespace mfrk::ui::flex::test;
unsigned differences(Rect a,Rect b){unsigned count=0;for(int j=0;j<4;++j)count+=std::bit_cast<std::uint32_t>(a[j])!=std::bit_cast<std::uint32_t>(b[j]);return count;}
}
int main(int argc,char**argv)try{if(argc!=3)throw std::runtime_error("usage: ui_flex_test kind fixture");require(std::fegetround()==FE_TONEAREST,"nearest rounding required");const auto&id=identity(argv[1]);auto r=Reader::file(argv[2],id);std::size_t bad=0,edge_bad=0,total=0;
if(id.kind=="flat"){const auto cases=read_flat(r,id.count);for(const auto&c:cases){auto actual=layout(c.box,c.items,c.area);if(actual.size()!=c.expected.size()){++bad;continue;}total+=actual.size();for(std::size_t j=0;j<actual.size();++j){bad+=differences(actual[j],c.expected[j]);edge_domain(actual[j]);auto ed=mfrk::ui::quantize_flex_edges(actual[j][0],actual[j][1],actual[j][2],actual[j][3]);for(int a=0;a<4;++a)edge_bad+=ed[a]!=c.edges[j][a];}}}
else{const auto cases=read_tree(r,id.count);for(const auto&c:cases){auto actual=layout_tree(c.nodes,c.area);if(actual.size()!=c.expected.size()){++bad;continue;}for(std::size_t n=0;n<actual.size();++n){if(actual[n].size()!=c.expected[n].size()){++bad;continue;}total+=actual[n].size();for(std::size_t i=0;i<actual[n].size();++i)bad+=differences(actual[n][i],c.expected[n][i]);}}}
std::cout<<id.kind<<": "<<id.count<<" original cases, "<<total<<" item rectangles, "<<bad<<" float differences, "<<edge_bad<<" edge differences\n";return bad||edge_bad?1:0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
