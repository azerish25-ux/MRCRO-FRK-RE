#include "mfrk/ui_scene_factory.hpp"
#include <limits>
#include <stdexcept>
namespace mfrk::ui::scene::factory {
namespace {
std::int32_t sum(std::int32_t first,std::int32_t second){
 const auto result=static_cast<std::int64_t>(first)+second;
 if(result<std::numeric_limits<std::int32_t>::min()||result>std::numeric_limits<std::int32_t>::max())throw std::invalid_argument("scene sibling coordinate admission");
 return static_cast<std::int32_t>(result);
}
bool metadata(std::string_view kind){
 constexpr std::string_view names[]={"graphics","multiple_params","input-submenu","output-submenu","submenu","signals","point","content","lookandfeel","project-save-popup"};
 return std::find(std::begin(names),std::end(names),kind)!=std::end(names);
}
}
std::uint64_t component_kind_hash(std::string_view kind)noexcept{
 std::uint64_t result=0x1505;
 for(const unsigned char byte:kind){const auto character=byte<128?static_cast<std::int64_t>(byte):static_cast<std::int64_t>(byte)-256;result=result*33+static_cast<std::uint64_t>(character);}
 return result;
}
void Builder::refresh_parent(GuiVisibilityComponent& parent,scope::Context& context){
 context.numbers["PW"]=parent.bounds[2];context.numbers["PH"]=parent.bounds[3];
 if(auto* previous=services_.previous_child(parent)){
  const auto b=previous->bounds;
  context.numbers["PXB"]=b[0];context.numbers["PXE"]=sum(b[2],b[0]);
  context.numbers["PYB"]=b[1];context.numbers["PYE"]=sum(b[1],b[3]);
  context.numbers["PXC"]=sum(b[2]/2,b[0]);context.numbers["PYC"]=sum(b[3]/2,b[1]);
 }
}
void Builder::component(const Node& node,GuiVisibilityComponent& parent,std::uint32_t depth,scope::Context& context){
 std::string kind=node.name;
 // Source helper9b1ed0 resolves first;9b2120 repeats only if it still says
 // component (for example a style explicitly resolving to that literal).
 for(unsigned pass=0;pass<2&&kind=="component";++pass){kind=services_.generic_component_kind(node,context);if(kind.empty())throw std::invalid_argument("empty generic component kind");}
 if(metadata(kind))return;
 if(kind=="xmlfile_component"){services_.include(node,parent,depth,context);return;}
 const auto raw_title=attribute_value("title",node.attributes,{});
 if(!raw_title)throw std::invalid_argument("component title required");
 const auto title=services_.expand_component_string(*raw_title,context);
 const auto created=services_.construct(node,kind,title,context);
 if(!created.component)throw std::invalid_argument("unknown component kind");
 services_.attach(parent,*created.component,node,context);
 if(created.build_children)children(node,*created.component,context,depth+1);
 services_.finish(*created.component);
}
void Builder::terminal(const Node& node,GuiVisibilityComponent& parent,std::uint32_t depth,scope::Context& context){
 if(node.name=="components")children(node,parent,context,depth);
 else if(node.name!="params")component(node,parent,depth,context);
 refresh_parent(parent,context);
}
void Builder::children(const Node& node,GuiVisibilityComponent& parent,scope::Context& context,std::uint32_t depth){
 if(depth>128)throw std::invalid_argument("component construction depth admission");
 context.numbers["PW"]=parent.bounds[2];context.numbers["PH"]=parent.bounds[3];
 scope::walk_nodes(node,context,[&](const Node& child,scope::Context& live){terminal(child,parent,depth,live);},3);
}
}
