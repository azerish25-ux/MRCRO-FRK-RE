#include "mfrk/ui_knob_feature.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
#include <iostream>
using namespace mfrk::ui;
struct Rows:GuiParameterObserver {
 void changed(std::uint32_t)override{throw std::logic_error("Unexpected notification");}
 void scalar_changed(std::uint32_t,float)override{throw std::logic_error("Unexpected scalar notification");}
};
struct Component:GuiKnobDisplayBoundary {
 GuiKnobDisplayGeometry style;std::vector<std::array<std::int32_t,4>> dirty;
 explicit Component(GuiKnobDisplayGeometry value):style(value){}
 GuiKnobDisplayGeometry geometry()override{return style;}
 void invalidate(const std::array<std::int32_t,4>& bounds)override{dirty.push_back(bounds);}
 void invalidate()override{throw std::logic_error("Unexpected full invalidation");}
 void set_component_state(bool)override{throw std::logic_error("Unexpected component state");}
};
int main(int argc,char** argv)try{
 if(argc!=2)return 2;
 using namespace mfrk::ui::scene::scope::test;
 constexpr Identity expected={"knob-scalar-dirty","KDSCAL01",26124,384,3637238918u,{0x9b,0xd9,0xd6,0x6d,0xa2,0x92,0x2d,0xee,0xa8,0x88,0x2d,0x6d,0x2f,0xfc,0x51,0x29,0x3f,0x44,0x79,0xfe,0x5b,0x21,0x47,0xd6,0xcc,0x5d,0xb,0xdc,0xfe,0x31,0xc,0xfb}};
 auto reader=Reader::file(argv[1],expected);unsigned differences=0;
 for(unsigned i=0;i<expected.count;++i){
  const auto width=reader.i32(),height=reader.i32();
  std::array<float,7> fields;for(auto& v:fields)v=reader.f32();
  std::vector<std::array<std::int32_t,4>> native(2);for(auto& rect:native)for(auto& v:rect)v=reader.i32();
  Rows rows;GuiControlParameters parameters(rows);
  Component component({width,height,fields[0],fields[1],fields[2],fields[3],fields[4]});
  GuiKnobFeatureBinding binding(parameters,component);binding.scalar_changed(fields[5],fields[6]);
  if(component.dirty!=native){if(differences++<5){std::cerr<<"Scalar dirty mismatch "<<i<<'\n';for(unsigned j=0;j<2;++j){std::cerr<<" actual:";for(auto v:component.dirty[j])std::cerr<<' '<<v;std::cerr<<" expected:";for(auto v:native[j])std::cerr<<' '<<v;std::cerr<<'\n';}}}
 }
 reader.end();std::cout<<"{\"cases\":"<<expected.count<<",\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
