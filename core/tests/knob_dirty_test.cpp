#include "mfrk/ui_knob_feature.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
struct Rows:GuiParameterObserver {
 void changed(std::uint32_t)override{throw std::logic_error("Unexpected notification");}
 void scalar_changed(std::uint32_t,float)override{throw std::logic_error("Unexpected scalar notification");}
};
struct Component:GuiKnobDisplayBoundary {
 GuiKnobDisplayGeometry style;std::array<std::int32_t,4> dirty{};unsigned calls=0;
 explicit Component(GuiKnobDisplayGeometry value):style(value){}
 GuiKnobDisplayGeometry geometry()override{return style;}
 void invalidate(const std::array<std::int32_t,4>& bounds)override{dirty=bounds;++calls;}
 void invalidate()override{throw std::logic_error("Unexpected full invalidation");}
 void scalar_changed(float,float)override{throw std::logic_error("Unexpected scalar callback");}
 void set_component_state(bool)override{throw std::logic_error("Unexpected component state");}
};
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;
 constexpr Identity expected={"knob-dirty","KNDIRTY1",40972,512,77914893u,{0x52,0xa5,0x45,0x66,0x4,0x62,0xec,0x6,0x30,0x69,0xf0,0xf,0xb8,0xed,0x1a,0xe2,0x88,0x51,0x25,0xe6,0x23,0xe8,0x19,0x96,0xad,0x6e,0x38,0x56,0xc9,0x29,0xf4,0x4f}};
 auto reader=Reader::file(argv[1],expected);unsigned differences=0;
 for(unsigned i=0;i<expected.count;++i){
  const auto width=reader.i32(),height=reader.i32();
  std::array<float,4> style;for(auto& v:style)v=reader.f32();
  std::array<float,5> old,current;for(auto& v:old)v=reader.f32();for(auto& v:current)v=reader.f32();
  std::array<std::int32_t,4> native;for(auto& v:native)v=reader.i32();
  Rows rows;GuiControlParameters parameters(rows);
  Component component({width,height,style[0],style[1],style[2],style[3]});
  GuiKnobFeatureBinding binding(parameters,component);binding.range_changed(old,current);
  if(component.calls!=1)throw std::runtime_error("Dirty scheduler callback count");
  const auto actual=component.dirty;
  if(actual!=native){if(differences++<5){std::cerr<<"Dirty mismatch "<<i<<" actual:";for(auto v:actual)std::cerr<<' '<<v;std::cerr<<" expected:";for(auto v:native)std::cerr<<' '<<v;std::cerr<<'\n';}}
 }
 reader.end();std::cout<<"{\"cases\":"<<expected.count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
