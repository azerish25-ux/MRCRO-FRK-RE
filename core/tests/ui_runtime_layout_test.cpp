#include "mfrk/ui_runtime_layout.hpp"
#include "ui_runtime_fixture_reader.hpp"
#include <cmath>
#include <iostream>
#include <string>
using namespace mfrk::ui::runtime;
using namespace mfrk::ui::runtime::test;
namespace {
bool same(float a,float b){return std::bit_cast<std::uint32_t>(a)==std::bit_cast<std::uint32_t>(b);}
bool same(double a,double b){return std::bit_cast<std::uint64_t>(a)==std::bit_cast<std::uint64_t>(b);}
template<class T,std::size_t N> void integers(Reader&r,std::array<T,N>&v){for(auto&x:v)x=r.i32();}
Geometry geometry(Reader&r){Geometry g;for(auto&x:g.relative)x=r.f64();for(auto&x:g.absolute)x=r.f64();return g;}
unsigned differences(const Geometry&a,const Geometry&b){unsigned n=0;for(unsigned j=0;j<4;++j){n+=!same(a.relative[j],b.relative[j]);n+=!same(a.absolute[j],b.absolute[j]);}return n;}
void finite_geometry(const Geometry&g){for(double v:g.relative)if(!std::isfinite(v))throw std::runtime_error("nonfinite fixture geometry");for(double v:g.absolute)if(!std::isfinite(v))throw std::runtime_error("nonfinite fixture geometry");}
struct GeometryCase {std::array<int,2>parent;std::array<int,4>child;Geometry expected;};
struct ChildCase {std::array<int,2>parent;int mode;bool context;float view,d4;Geometry g;bool write;std::array<int,4>expected;};
struct RuntimeGeometryCase {std::array<float,2>background;float view;std::array<int,4>bounds;Geometry expected;};
struct LayoutCase {RuntimeLayoutState state;std::array<int,2>expected_parent{},expected_limits{};std::vector<float>cache;std::vector<std::array<int,4>>bounds;std::vector<Geometry>expected;};
LayoutCase layout_case(Reader&r,bool event,bool four_flags){LayoutCase c;auto&a=c.state.layout;a.horizontal=r.flag();integers(r,a.parent_dimensions);for(auto&v:a.background_dimensions)v=r.f32();a.view_scale=r.f32();integers(r,a.owner_cc_d0);if(event){c.state.enabled=r.flag();if(four_flags)a.fill_cross=false;}else a.fill_cross=r.flag();const auto n=r.u32();if(n>256)throw std::runtime_error("fixture item count");if(event){integers(r,c.expected_parent);integers(r,c.expected_limits);}for(unsigned i=0;i<n;++i){RuntimeLayoutItem x;integers(r,x.bounds);integers(r,x.reference_bounds);x.cached_length=r.f32();x.view_scale=r.f32();x.visible=r.flag();x.fixed=r.flag();x.keep_ratio=r.flag();if(four_flags)x.opaque_flag_a=r.flag();x.fixed_current=r.flag();x.minimum=r.i32();x.maximum=r.i32();c.state.items.push_back(x);c.state.geometry.emplace_back();c.cache.push_back(r.f32());std::array<int,4>b;integers(r,b);c.bounds.push_back(b);c.expected.push_back(geometry(r));finite_geometry(c.expected.back());if(!std::isfinite(x.cached_length)||!std::isfinite(x.view_scale)||!std::isfinite(c.cache.back()))throw std::runtime_error("nonfinite fixture cache");}return c;}
}
int main(int argc,char**argv)try{
 if(argc!=3)throw std::runtime_error("kind and fixture required");const std::string kind=argv[1];const auto&id=identity(kind);auto r=Reader::file(argv[2],id);unsigned bad=0;std::size_t total=0;
 // Each complete authenticated payload is decoded and checked to EOF before the
 // first numerical call. The exact identity is not replaceable by input metadata.
 if(kind=="compile"){std::vector<GeometryCase>cases;for(unsigned c=0;c<id.count;++c){GeometryCase x;integers(r,x.parent);integers(r,x.child);x.expected=geometry(r);finite_geometry(x.expected);cases.push_back(x);}r.end();for(const auto&x:cases)bad+=differences(compile_geometry(x.parent,x.child),x.expected);}
 else if(kind=="child"){std::vector<ChildCase>cases;for(unsigned c=0;c<id.count;++c){ChildCase x;integers(r,x.parent);x.mode=r.i32();x.context=r.flag();x.view=r.f32();x.d4=r.f32();x.g=geometry(r);x.write=r.flag();integers(r,x.expected);finite_geometry(x.g);if(!std::isfinite(x.view)||!std::isfinite(x.d4))throw std::runtime_error("fixture scale");cases.push_back(x);}r.end();for(const auto&x:cases){const auto actual=resize_child(x.parent,x.g,x.mode,x.context,x.view,x.d4);bad+=actual.has_value()!=x.write;if(actual)for(unsigned j=0;j<4;++j)bad+=(*actual)[j]!=x.expected[j];}}
 else if(kind=="geometry"){std::vector<RuntimeGeometryCase>cases;for(unsigned c=0;c<id.count;++c){RuntimeGeometryCase x;for(auto&v:x.background)v=r.f32();x.view=r.f32();integers(r,x.bounds);x.expected=geometry(r);finite_geometry(x.expected);cases.push_back(x);}r.end();for(const auto&x:cases)bad+=differences(update_runtime_geometry(x.background,x.view,x.bounds),x.expected);}
 else if(kind=="layout"||kind=="event"||kind=="event-current"||kind=="layout-flags"||kind=="event-flags"){const bool event=kind=="event"||kind=="event-current"||kind=="event-flags";const bool four_flags=kind=="layout-flags"||kind=="event-flags";std::vector<LayoutCase>cases;for(unsigned c=0;c<id.count;++c)cases.push_back(layout_case(r,event,four_flags));r.end();for(auto&c:cases){auto&s=c.state;if(event){parent_resized(s);bad+=s.layout.parent_dimensions!=c.expected_parent||s.layout.owner_cc_d0!=c.expected_limits;}else s.geometry=update_layout(s.layout,s.items);total+=s.items.size();for(unsigned i=0;i<s.items.size();++i){const bool different=!same(s.items[i].cached_length,c.cache[i])||s.items[i].bounds!=c.bounds[i]||differences(s.geometry[i],c.expected[i])!=0;if(different){if(bad<8)std::cerr<<"item "<<i<<" flags "<<s.items[i].fixed<<s.items[i].keep_ratio<<s.items[i].opaque_flag_a<<s.items[i].fixed_current<<" cache "<<s.items[i].cached_length<<" expected "<<c.cache[i]<<" boundsDiff "<<(s.items[i].bounds!=c.bounds[i])<<" geometryDiff "<<differences(s.geometry[i],c.expected[i])<<"\n";++bad;}}}}
 else if(kind=="math"){std::vector<std::array<std::uint32_t,3>>cases;for(unsigned c=0;c<id.count;++c){std::array<std::uint32_t,3>x{r.u32(),r.u32(),r.u32()};for(auto bits:x)if((bits&0x7fffffffu)>=0x7f800000u)throw std::runtime_error("nonfinite math fixture");cases.push_back(x);}r.end();for(auto x:cases){const auto v=std::bit_cast<float>(x[0]);bad+=std::bit_cast<std::uint32_t>(native_round(v))!=x[1]||std::bit_cast<std::uint32_t>(native_trunc(v))!=x[2];}}
 else throw std::runtime_error("unsupported kind");
 std::cout<<kind<<": "<<id.count<<" original cases, "<<total<<" items, "<<bad<<" differences\n";return bad?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
