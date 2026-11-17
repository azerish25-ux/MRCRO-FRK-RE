#include "mfrk/ui_scene_factory.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace mfrk::ui;
using namespace mfrk::ui::scene;
struct Driver:factory::Services {
 std::vector<std::unique_ptr<GuiVisibilityComponent>> owned;
 std::map<GuiVisibilityComponent*,std::string> names;
 std::map<GuiVisibilityComponent*,GuiVisibilityComponent*> previous;
 std::vector<std::string> events;
 std::string generic_component_kind(const factory::Node& n,scope::Context& c)override{
  const auto value=attribute_value("component",n.attributes,{});
  return value?expand_component_string(*value,c):"smartcomponent";
 }
 std::string expand_component_string(std::string_view text,scope::Context& c)override{return expand_aliases(text,c.strings,true);}
 factory::Construction construct(const factory::Node& node,std::string_view kind,std::string_view title,scope::Context&)override{
  if(kind!="view"&&kind!="knob"&&kind!="smartcomponent")return {nullptr,false};
  auto component=std::make_unique<GuiVisibilityComponent>();auto* result=component.get();owned.push_back(std::move(component));names.emplace(result,title);events.push_back("construct:"+std::string(title));
  return {result,!attribute_value("opaque",node.attributes,{})};
 }
 void attach(GuiVisibilityComponent& parent,GuiVisibilityComponent& child,const factory::Node& node,scope::Context& c)override{
  DeclaredGeometryInput input;input.parent_kind=DeclaredParent::smart;input.parent_dimensions={parent.bounds[2],parent.bounds[3]};input.initial_check_bounds=false;
  if(auto* last=previous_child(parent))input.previous=last->bounds;
  const auto geometry=compile_declared_geometry(node.attributes,{},c.numbers,input,c.strings,true,c.features);
  if(!geometry.bounds)throw std::runtime_error("expected geometry");
  child.bounds=*geometry.bounds;child.parent=&parent;parent.children.push_back(&child);previous[&parent]=&child;events.push_back("attach:"+names.at(&child));
 }
 void finish(GuiVisibilityComponent& child)override{events.push_back("finish:"+names.at(&child));if(names.at(&child)=="Visu")++child.bounds[3];}
 GuiVisibilityComponent* previous_child(GuiVisibilityComponent& parent)override{const auto found=previous.find(&parent);return found==previous.end()?nullptr:found->second;}
 void include(const factory::Node&,GuiVisibilityComponent& parent,std::uint32_t depth,scope::Context&)override{if(depth!=0)throw std::runtime_error("include depth");parent.bounds[2]=900;events.push_back("include");}
 GuiVisibilityComponent& named(std::string_view name){for(const auto& [node,title]:names)if(title==name)return *node;throw std::runtime_error("missing widget");}
};
int main()try{
 const auto document=structure::parse_native_xml("<root><view title='Visu' x='0' y='100' w='500' h='2.3*PH/5'><knob title='Inside' x='10' y='15' w='125' h='125'/></view><params/><with src='Id' string='Tail'><knob title='${Id}' x='0' y='PYE+10' w='PW/10' h='50'/></with><graphics><unsupported/></graphics><component component='knob' title='Generic' x='0' y='PYE' w='100' h='20'/><xmlfile_component/><knob title='After' x='0' y='PYE' w='PW/10' h='10'/><view title='Opaque' opaque='1' x='0' y='0' w='50' h='50'><unsupported/></view></root>");
 Driver driver;factory::Builder builder(driver);GuiVisibilityComponent parent;parent.bounds={0,0,1000,575};scope::Context context;
 builder.children(document.children.at(0),parent,context);
 const std::vector<std::string> expected={"construct:Visu","attach:Visu","construct:Inside","attach:Inside","finish:Inside","finish:Visu","construct:Tail","attach:Tail","finish:Tail","construct:Generic","attach:Generic","finish:Generic","include","construct:After","attach:After","finish:After","construct:Opaque","attach:Opaque","finish:Opaque"};
 if(driver.events!=expected||parent.children.size()!=5||driver.named("Visu").bounds!=GuiDirtyBounds{0,100,500,265}||driver.named("Inside").bounds!=GuiDirtyBounds{10,15,125,125}||driver.named("Tail").bounds!=GuiDirtyBounds{0,375,100,50}||driver.named("Generic").bounds!=GuiDirtyBounds{0,425,100,20}||driver.named("After").bounds!=GuiDirtyBounds{0,445,90,10}||context.strings.contains("Id")||context.numbers.at("PW")!=900||context.numbers.at("PXC")!=25)throw std::runtime_error("source scene construction ordering/geometry");
 unsigned rejections=0;
 for(const auto text:{"<root><knob/></root>","<root><unknown title='No'/></root>","<root><component component='' title='No'/></root>"}){const auto bad=structure::parse_native_xml(text);try{builder.children(bad.children.at(0),parent,context);}catch(const std::invalid_argument&){++rejections;}}
 if(rejections!=3)throw std::runtime_error("unknown/missing constructor admission");
 // Expected values execute original factory instructions9b2a30..9b2a73;
 // the empty path stops at its actual extension dispatch9b920c.
 if(factory::component_kind_hash("view")!=0x17c9f9060ULL||factory::component_kind_hash("panel")!=0x311020cab5ULL||factory::component_kind_hash("softknob")!=0x1ae78af4241c8bULL||factory::component_kind_hash("smartcomponent")!=0x8ee3f0648a8f181fULL||factory::component_kind_hash(std::string_view("\x80\xff",2))!=0x5959c4ULL||factory::component_kind_hash("")!=0x1505ULL)throw std::runtime_error("factory tag hash");
 std::cout<<"Scene construction: 6 widgets, 19 ordered construction events, scoped layout, 3 explicit rejections and 6 original tag hashes passed\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
