#include "mfrk/ui.hpp"
#include "ui_fixture_reader.hpp"
#include "ui_fixture_domains.hpp"
#include <cfenv>
#include <iostream>
namespace {
using namespace mfrk::ui;using namespace mfrk::ui::test;
bool same(std::span<const float>a,std::span<const float>b){if(a.size()!=b.size())return false;for(std::size_t i=0;i<a.size();++i)if(std::bit_cast<std::uint32_t>(a[i])!=std::bit_cast<std::uint32_t>(b[i]))return false;return true;}
std::array<float,4> expected4(Reader&r){std::array<float,4>v;for(auto&x:v){x=r.f32();require(finite(x),"fixture finite expected point");}return v;}
}
int main(int argc,char**argv)try{
 if(argc!=3)throw std::runtime_error("usage: ui_primitives_test kind fixture");require(std::fegetround()==FE_TONEAREST,"fixture requires nearest rounding");const auto&id=identity(argv[1]);auto r=Reader::file(argv[2],id);std::size_t bad=0;
 for(std::uint32_t i=0;i<id.count;++i){
  if(id.kind=="center"){auto w=r.i32(),h=r.i32();std::array<float,5>s;for(auto&x:s)x=r.f32();auto expected=expected4(r);center_domain(w,h,s);auto actual=interpolated_center_bounds(w,h,s[0],s[1],s[2],s[3],s[4]);bad+=!same(actual,expected);}
  else if(id.kind=="trig"){float x=r.f32();std::array<float,2>expected;for(auto&v:expected){v=r.f32();require(finite(v),"fixture finite expected trig lane");}trig_argument(x);auto actual=packed_sine_cosine(x);bad+=!same(actual,expected);}
  else if(id.kind=="gradient"){auto w=r.i32(),h=r.i32();std::array<float,3>s;for(auto&x:s)x=r.f32();auto expected=expected4(r);gradient_domain(w,h,s);const float radius=.69f*static_cast<float>(std::min(w/2,h/2));auto actual=outline_gradient_points(w,h,s[0],s[1],s[2],radius);bad+=!same(actual,expected);}
  else if(id.kind=="marker"){auto w=r.i32(),h=r.i32();std::array<float,4>s;for(auto&x:s)x=r.f32();auto expected=expected4(r);marker_domain(w,h,s);float radius=.69f*static_cast<float>(std::min(w/2,h/2));auto actual=center_marker_points(w,h,s[0],s[1]!=0,2.4f,2.4f,radius,s[2],s[3]);bad+=!same(actual,expected);}
  else if(id.kind=="magnetic"){auto n=r.u32();require(n<=100,"fixture zone count");std::vector<MagneticZone>zones;zones.reserve(n);for(std::uint32_t j=0;j<n;++j){MagneticZone z;z.position_from=r.f32();z.position_to=r.f32();z.value_from=r.f32();z.value_to=r.f32();z.skip_at_drag_start=r.flag();zones.push_back(z);}float value=r.f32(),delta=r.f32();bool a=r.flag(),b=r.flag();auto expected=r.u32();bool skip=r.flag();require(finite(std::bit_cast<float>(expected)),"fixture finite expected magnetic value");magnetic_domain(zones,value,delta);auto actual=magnetic_delta(zones,value,delta,a,b);bad+=std::bit_cast<std::uint32_t>(actual.value)!=expected||actual.skip!=skip;}
  else if(id.kind=="flex"){std::array<float,4>v;for(auto&x:v)x=r.f32();std::array<std::int32_t,4>expected;for(auto&x:expected)x=r.i32();flex_domain(v);auto actual=quantize_flex_edges(v[0],v[1],v[2],v[3]);bad+=actual!=expected;}
  else if(id.kind=="scaled"){float scale=r.f32();double logical=r.f64();auto expected=r.i32();scaled_domain(scale,logical);bad+=scaled_absolute_dimension(scale,logical)!=expected;}
 }
 r.end();std::cout<<id.kind<<": "<<id.count<<" original cases, "<<bad<<" mismatches\n";return bad?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
