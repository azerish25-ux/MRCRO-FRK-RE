#include "mfrk/ui_runtime_layout.hpp"
#include <bit>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mfrk::ui::runtime;
int main()try{unsigned rejected=0;auto bad=[&](auto f){try{f();}catch(const std::invalid_argument&){++rejected;return;}throw std::runtime_error("unsupported domain accepted");};const float nan=std::numeric_limits<float>::quiet_NaN();
 bad([&]{compile_geometry({0,100},{0,0,50,50});});bad([&]{compile_geometry({1000001,100},{0,0,50,50});});bad([&]{compile_geometry({100,100},{1000001,0,50,50});});
 bad([&]{resize_child({100,100},{},4,true,1,1);});bad([&]{resize_child({100,100},{},1,true,nan,1);});bad([&]{Geometry g;g.relative[0]=1e6;resize_child({1000000,100},g,1,true,1,1);});
 bad([&]{update_runtime_geometry({0,100},1,{0,0,50,50});});bad([&]{update_runtime_geometry({100,100},0,{0,0,50,50});});bad([&]{native_round(nan);});bad([&]{native_trunc(std::numeric_limits<float>::infinity());});
 RuntimeLayout a;a.parent_dimensions={100,100};a.background_dimensions={100,100};RuntimeLayoutItem x;x.bounds={0,0,50,50};x.reference_bounds=x.bounds;x.cached_length=50;
 bad([&]{auto v=x;v.bounds[2]=-1;std::vector<RuntimeLayoutItem>items{v};update_layout(a,items);});bad([&]{auto v=x;v.reference_bounds[3]=-1;std::vector<RuntimeLayoutItem>items{v};update_layout(a,items);});bad([&]{auto v=x;v.maximum=-1;std::vector<RuntimeLayoutItem>items{v};update_layout(a,items);});bad([&]{auto v=x;v.cached_length=nan;std::vector<RuntimeLayoutItem>items{v};update_layout(a,items);});bad([&]{auto v=x;v.cached_length=-1;std::vector<RuntimeLayoutItem>items{v};update_layout(a,items);});bad([&]{std::vector<RuntimeLayoutItem>items(257,x);update_layout(a,items);});bad([&]{auto v=a;v.parent_dimensions[0]=-1;std::vector<RuntimeLayoutItem>items{x};update_layout(v,items);});
 bad([&]{RuntimeLayoutState s{a,{x},{},true};parent_resized(s);});bad([&]{auto invalid=a;invalid.view_scale=0;std::vector<RuntimeLayoutItem>items{x};update_layout(invalid,items);});bad([&]{RuntimeLayoutState s{a,{}, {},true};parent_resized(s);});
 // Mode0 has no resize action and intentionally does not inspect unused fields.
 if(resize_child({-1,-1},{},0,true,nan,nan))throw std::runtime_error("mode0 wrote");
 x.fixed_current=true;RuntimeLayoutState disabled{a,{x},{Geometry{}},false};disabled.geometry[0].absolute[0]=-0.;parent_resized(disabled);if(disabled.layout.parent_dimensions!=a.parent_dimensions||disabled.items[0].bounds!=x.bounds||disabled.items[0].cached_length!=50||std::bit_cast<std::uint64_t>(disabled.geometry[0].absolute[0])!=0x8000000000000000ull)throw std::runtime_error("disabled event changed state");
 for(auto fn:{native_round,native_trunc})if(std::bit_cast<std::uint32_t>(fn(-0.f))!=0x80000000u)throw std::runtime_error("lost signed zero");
 std::cout<<rejected<<" guards rejected; disabled state/mode0/signed zero preserved\n";return rejected==20?0:1;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
