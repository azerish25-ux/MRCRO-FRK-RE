#pragma once
#include "mfrk/ui_mouse_source.hpp"
#include "mfrk/ui_component_focus.hpp"
namespace mfrk::ui {
struct GuiPointerPeer:GuiFocusPeer {
 virtual GuiVisibilityComponent& root_component()=0;
 virtual GuiMousePoint local_to_global(GuiMousePoint)=0;
 virtual GuiMousePoint global_to_local(GuiMousePoint)=0;
 virtual bool contains(std::int32_t x,std::int32_t y,bool include_children)=0;
 virtual std::uint32_t identifier()=0;
};
struct GuiPointerGeometryBoundary {
 virtual ~GuiPointerGeometryBoundary()=default;
 virtual GuiPointerPeer* peer(GuiVisibilityComponent&)=0;
 virtual bool valid_peer(GuiPointerPeer*)=0;
 virtual float component_scale(GuiVisibilityComponent&)=0;
 virtual float desktop_scale()=0;
 virtual bool hit_test(GuiVisibilityComponent&,std::int32_t x,std::int32_t y)=0;
};
inline GuiMousePoint gui_pointer_affine(GuiMousePoint p,const std::array<float,6>& m){
 return {(p.y*m[1]+p.x*m[0])+m[2],(p.x*m[3]+p.y*m[4])+m[5]};
}
// Source f81c50. The determinant is rounded to float before the double inverse;
// singular finite matrices return the original matrix rather than identity.
inline std::array<float,6> gui_pointer_inverse(const std::array<float,6>& m){
 const float determinant=m[0]*m[4]-m[1]*m[3];
 if(determinant==0.f)return m;
 const double inverse=1./static_cast<double>(determinant);
 const float a=static_cast<float>(static_cast<double>(m[4])*inverse),e=static_cast<float>(static_cast<double>(m[0])*inverse);
 const float d=static_cast<float>(static_cast<double>(-m[3])*inverse),b=static_cast<float>(static_cast<double>(-m[1])*inverse);
 return {a,b,(-m[2]*a)-m[5]*b,d,e,(-m[2]*d)-m[5]*e};
}
inline GuiMousePoint gui_pointer_scale(GuiMousePoint p,float scale){if(scale!=1.f){p.x=p.x*scale;p.y=scale*p.y;}return p;}
inline GuiMousePoint gui_pointer_unscale(GuiMousePoint p,float scale){if(scale!=1.f){const float inverse=1.f/scale;p.x=inverse*p.x;p.y=inverse*p.y;}return p;}
inline GuiPointerPeer* gui_pointer_peer(GuiVisibilityComponent& component,GuiPointerGeometryBoundary& boundary){
 for(auto* node=&component;node;node=static_cast<GuiVisibilityComponent*>(node->parent))if((node->flags&1u)!=0)return boundary.peer(*node);
 return nullptr;
}
// Source108da30 plus the caller's affine step. Root peer/scale operations are
// real platform services. Ordinary children first add their integer position.
inline GuiMousePoint gui_pointer_to_parent(GuiVisibilityComponent& component,GuiMousePoint p,GuiPointerGeometryBoundary& boundary){
 if((component.flags&1u)==0){
  if(component.parent)p={static_cast<float>(component.bounds[0])+p.x,static_cast<float>(component.bounds[1])+p.y};
  else{
   p={static_cast<float>(component.bounds[0])+p.x,p.y+static_cast<float>(component.bounds[1])};
   p=gui_pointer_scale(p,boundary.component_scale(component));p=gui_pointer_unscale(p,boundary.desktop_scale());
  }
 }else if(auto* peer=boundary.peer(component)){
  p=gui_pointer_scale(p,boundary.component_scale(component));p=peer->local_to_global(p);p=gui_pointer_unscale(p,boundary.desktop_scale());
 }
 if(component.transform)p=gui_pointer_affine(p,*component.transform);
 return p;
}
// Source108dbe0/ec90. Root-without-peer returns the inverse-transformed point.
inline GuiMousePoint gui_pointer_from_parent(GuiVisibilityComponent& component,GuiMousePoint p,GuiPointerGeometryBoundary& boundary){
 if(component.transform)p=gui_pointer_affine(p,gui_pointer_inverse(*component.transform));
 if((component.flags&1u)==0){
  if(!component.parent){p=gui_pointer_scale(p,boundary.desktop_scale());const float scale=boundary.component_scale(component);if(scale!=1.f){p.x=p.x/scale;p.y=p.y/scale;}}
  return {p.x-static_cast<float>(component.bounds[0]),p.y-static_cast<float>(component.bounds[1])};
 }
 if(auto* peer=boundary.peer(component)){
  p=gui_pointer_scale(p,boundary.desktop_scale());p=peer->global_to_local(p);p=gui_pointer_unscale(p,boundary.component_scale(component));
 }
 return p;
}
inline GuiMousePoint gui_pointer_from_ancestor(GuiVisibilityComponent* ancestor,GuiVisibilityComponent& target,GuiMousePoint p,GuiPointerGeometryBoundary& boundary){
 if(target.parent!=ancestor)p=gui_pointer_from_ancestor(ancestor,*static_cast<GuiVisibilityComponent*>(target.parent),p,boundary);
 return gui_pointer_from_parent(target,p,boundary);
}
// Source108d350/10911f0. Null source means Desktop/global logical space.
inline GuiMousePoint gui_pointer_map_point(GuiVisibilityComponent* target,GuiVisibilityComponent* source,GuiMousePoint p,GuiPointerGeometryBoundary& boundary){
 for(auto* from=source;from;from=static_cast<GuiVisibilityComponent*>(from->parent)){
  if(from==target)return p;
  for(auto* node=target?static_cast<GuiVisibilityComponent*>(target->parent):nullptr;node;node=static_cast<GuiVisibilityComponent*>(node->parent))if(node==from){
   return gui_pointer_from_ancestor(from,*target,p,boundary);
  }
  p=gui_pointer_to_parent(*from,p,boundary);
 }
 if(!target)return p;
 auto* root=target;while(root->parent)root=static_cast<GuiVisibilityComponent*>(root->parent);
 p=gui_pointer_from_parent(*root,p,boundary);
 return root==target?p:gui_pointer_from_ancestor(root,*target,p,boundary);
}
inline GuiMousePoint gui_pointer_local_point(GuiVisibilityComponent& target,GuiVisibilityComponent* source,GuiMousePoint p,GuiPointerGeometryBoundary& boundary){
 return gui_pointer_map_point(&target,source,p,boundary);
}
// Source a67f10: the source binary64 addition/low-word extraction retains ties.
inline std::int32_t gui_pointer_round(float value){
 const double shifted=static_cast<double>(value)+6755399441055744.;
 return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(std::bit_cast<std::uint64_t>(shifted)));
}
inline bool gui_pointer_in_bounds(GuiVisibilityComponent& component,GuiMousePoint p,GuiPointerGeometryBoundary& boundary){
 const auto x=gui_pointer_round(p.x),y=gui_pointer_round(p.y);
 if(static_cast<std::uint32_t>(component.bounds[2])<=static_cast<std::uint32_t>(x)||static_cast<std::uint32_t>(component.bounds[3])<=static_cast<std::uint32_t>(y))return false;
 return boundary.hit_test(component,x,y);
}
// Source ff4ed0, including ancestor clipping and the actual root peer check.
inline bool gui_pointer_contains(GuiVisibilityComponent& component,GuiMousePoint p,GuiPointerGeometryBoundary& boundary){
 auto* node=&component;
 for(;;){
  if(!gui_pointer_in_bounds(*node,p,boundary))return false;
  if(!node->parent)break;
  p=gui_pointer_to_parent(*node,p,boundary);node=static_cast<GuiVisibilityComponent*>(node->parent);
 }
 if((node->flags&1u)!=0)if(auto* peer=boundary.peer(*node)){
  if(node->transform)p=gui_pointer_affine(p,*node->transform);
  p=gui_pointer_scale(p,boundary.component_scale(*node));return peer->contains(gui_pointer_round(p.x),gui_pointer_round(p.y),true);
 }
 return false;
}
// Source ff5210. A stable child range is traversed in reverse paint order.
inline GuiVisibilityComponent* gui_pointer_component_at(GuiVisibilityComponent& component,GuiMousePoint p,GuiPointerGeometryBoundary& boundary){
 if((component.flags&2u)==0||!gui_pointer_in_bounds(component,p,boundary))return nullptr;
 const auto count=component.children.size();
 for(std::size_t i=count;i>0;--i){auto* child=component.children[i-1];const auto local=gui_pointer_from_parent(*child,p,boundary);if(auto* found=gui_pointer_component_at(*child,local,boundary))return found;}
 return &component;
}
// Source ffa390 uses division, while ffa210 uses rounded reciprocal multiply.
inline GuiVisibilityComponent* gui_mouse_find_target(GuiMouseSourceState& state,GuiMousePoint screen,GuiPointerGeometryBoundary& boundary){
 if(!boundary.valid_peer(state.peer)){state.peer=nullptr;return nullptr;}
 if(auto* peer=state.peer){
  auto* captured=&peer->root_component();auto p=peer->global_to_local(screen);const float scale=boundary.component_scale(*captured);
  if(scale!=1.f){p.x=p.x/scale;p.y=p.y/scale;}
  auto& root=peer->root_component();if(gui_pointer_contains(root,p,boundary))return gui_pointer_component_at(root,p,boundary);
 }
 return nullptr;
}
inline GuiMousePoint gui_mouse_local_position(GuiVisibilityComponent& component,GuiMousePoint screen,GuiPointerGeometryBoundary& boundary){
 if(auto* peer=gui_pointer_peer(component,boundary)){
  auto p=peer->global_to_local(screen);auto& root=peer->root_component();p=gui_pointer_unscale(p,boundary.component_scale(root));return gui_pointer_local_point(component,&root,p,boundary);
 }
 return gui_pointer_local_point(component,nullptr,gui_pointer_unscale(screen,boundary.component_scale(component)),boundary);
}
// Concrete coordinate/hit services for the pointer owner. Other component
// event, clock/cursor and platform operations remain actual derived services.
struct GuiMouseGeometryBinding:GuiMouseInputBoundary {
 GuiPointerGeometryBoundary& geometry;
 explicit GuiMouseGeometryBinding(GuiPointerGeometryBoundary& boundary):geometry(boundary){}
 GuiVisibilityComponent* find_target(GuiMouseSourceState& state,GuiMousePoint p)final{return gui_mouse_find_target(state,p,geometry);}
 GuiMousePoint local_position(GuiVisibilityComponent& node,GuiMousePoint p)final{return gui_mouse_local_position(node,p,geometry);}
 std::uint32_t peer_identifier(GuiVisibilityComponent& node)final{auto* peer=gui_pointer_peer(node,geometry);return peer?peer->identifier():0;}
};
// Callers retain strict nearest FP/no contraction, finite scales/coordinates/
// matrices/intermediates, representable rounded points and nonnegative bounds.
// Trees are acyclic, ancestors are complete GuiVisibilityComponent nodes, and
// child storage/peer implementations survive the current service traversal.
}
