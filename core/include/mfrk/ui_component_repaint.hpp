#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
namespace mfrk::ui {
// Compile callers with strict floating point and contraction disabled.
using GuiDirtyBounds=std::array<std::int32_t,4>;
struct GuiRepaintGate {
 virtual ~GuiRepaintGate()=default;
 // Source gate v18 for ordinary repaint, v10 for the immediate path.
 virtual bool allow(bool immediate)=0;
};
struct GuiRepaintComponent {
 std::uint8_t flags;
 GuiDirtyBounds bounds;
 GuiRepaintComponent* parent;
 const std::array<float,6>* transform;
 GuiRepaintGate* gate;
};
struct GuiRepaintPeer {
 virtual ~GuiRepaintPeer()=default;
 virtual GuiDirtyBounds client_bounds()=0;
 virtual void invalidate(const GuiDirtyBounds&)=0;
};
struct GuiRepaintBoundary {
 virtual ~GuiRepaintBoundary()=default;
 // Source component v90; returned coordinates are in the parent's space.
 virtual GuiDirtyBounds visible_bounds(GuiRepaintComponent&)=0;
 // Source 1068560; may return null when the actual peer is absent.
 virtual GuiRepaintPeer* peer(GuiRepaintComponent&)=0;
};
// Source 8503c0 admitted finite/representable profile. All endpoints and their
// integer differences must fit signed32, including float-rounded edge sums.
inline GuiDirtyBounds gui_dirty_outward(float x,float y,float width,float height){
 const auto left=static_cast<std::int32_t>(std::floor(x));
 const auto top=static_cast<std::int32_t>(std::floor(y));
 const auto right=static_cast<std::int32_t>(std::ceil(x+width));
 const auto bottom=static_cast<std::int32_t>(std::ceil(y+height));
 return {left,top,right-left,bottom-top};
}
// Source b7d8b0: four transformed corners, retaining native addition order.
inline GuiDirtyBounds gui_dirty_transform(const GuiDirtyBounds& r,const std::array<float,6>& m){
 const float x=static_cast<float>(r[0]),y=static_cast<float>(r[1]);
 const float right=static_cast<float>(r[0]+r[2]),bottom=static_cast<float>(r[1]+r[3]);
 const float ax=m[0]*x,ar=m[0]*right,by=m[1]*y,bb=m[1]*bottom;
 const float dx=m[3]*x,dr=m[3]*right,ey=m[4]*y,eb=m[4]*bottom;
 const std::array<float,4> xs{(ax+by)+m[2],(ar+by)+m[2],(bb+ax)+m[2],(bb+ar)+m[2]};
 const std::array<float,4> ys{(ey+dx)+m[5],(dr+ey)+m[5],(eb+dx)+m[5],(eb+dr)+m[5]};
 const float left=*std::min_element(xs.begin(),xs.end()),top=*std::min_element(ys.begin(),ys.end());
 const float right_edge=*std::max_element(xs.begin(),xs.end()),bottom_edge=*std::max_element(ys.begin(),ys.end());
 return gui_dirty_outward(left,top,right_edge-left,bottom_edge-top);
}
inline void gui_component_repaint(GuiRepaintComponent&,GuiDirtyBounds&,GuiRepaintBoundary&);
// Source ff6120. Live serial component/peer/gate ownership and acyclic parents
// are required. Integer translations/edges must not overflow; finite affine
// fields and nonzero peer-component dimensions when a positive root request
// reaches the peer. Platform peer operations are mandatory actual services.
inline void gui_component_propagate_repaint(GuiRepaintComponent& component,
 const GuiDirtyBounds& request,bool immediate,GuiRepaintBoundary& boundary){
 if((component.flags&2u)==0)return;
 if(auto* gate=component.gate)if(!gate->allow(immediate))return;
 if(request[2]<=0||request[3]<=0)return;
 if((component.flags&1u)==0){
  if(auto* parent=component.parent){
   auto translated=request;translated[0]+=component.bounds[0];translated[1]+=component.bounds[1];
   if(component.transform)translated=gui_dirty_transform(translated,*component.transform);
   gui_component_repaint(*parent,translated,boundary);
  }
 }else if(auto* peer=boundary.peer(component)){
  const auto client=peer->client_bounds();
  const float sx=static_cast<float>(client[2])/static_cast<float>(component.bounds[2]);
  const float sy=static_cast<float>(client[3])/static_cast<float>(component.bounds[3]);
  auto scaled=gui_dirty_outward(static_cast<float>(request[0])*sx,static_cast<float>(request[1])*sy,
                               static_cast<float>(request[2])*sx,static_cast<float>(request[3])*sy);
  if(component.transform)scaled=gui_dirty_transform(scaled,*component.transform);
  peer->invalidate(scaled);
 }
}
// Source ff6020: intersect the caller's rectangle in-place, then propagate
// only positive area. Zero-area touching edges retain their source location;
// disjoint rectangles become {0,0,0,0} even when component repaint is disabled.
inline void gui_component_repaint(GuiRepaintComponent& component,GuiDirtyBounds& request,GuiRepaintBoundary& boundary){
 const auto visible=boundary.visible_bounds(component);
 const auto local_x=visible[0]-component.bounds[0],local_y=visible[1]-component.bounds[1];
 const auto x=std::max(request[0],local_x),y=std::max(request[1],local_y);
 const auto right=std::min(request[0]+request[2],local_x+visible[2]);
 const auto bottom=std::min(request[1]+request[3],local_y+visible[3]);
 if(right-x<0||bottom-y<0)request={0,0,0,0};
 else request={x,y,right-x,bottom-y};
 if(request[2]>0&&request[3]>0)gui_component_propagate_repaint(component,request,false,boundary);
}
// Source ff5ee0: use the actual visible bounds, subtract current component
// position and enter the immediate gate path without an extra clipping call.
inline void gui_component_full_repaint(GuiRepaintComponent& component,GuiRepaintBoundary& boundary){
 auto request=boundary.visible_bounds(component);
 request[0]-=component.bounds[0];request[1]-=component.bounds[1];
 gui_component_propagate_repaint(component,request,true,boundary);
}
}
