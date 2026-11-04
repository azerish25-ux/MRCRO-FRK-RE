#include "mfrk/ui_runtime_layout.hpp"
#include <bit>
#include <cmath>
#include <stdexcept>
namespace mfrk::ui::runtime {
Geometry compile_geometry(std::array<int,2>parent,std::array<int,4>child){
 if(parent[0]<=0||parent[1]<=0||parent[0]>1000000||parent[1]>1000000)throw std::invalid_argument("reference parent dimensions");
 for(int v:child)if(v<-1000000||v>1000000)throw std::invalid_argument("reference child bounds");
 const float inverse_h=1.f/float(parent[1]),inverse_w=1.f/float(parent[0]);Geometry result;
 result.relative={double(float(child[0])*inverse_w),double(float(child[1])*inverse_h),double(float(child[2])*inverse_w),double(float(child[3])*inverse_h)};
 for(int i=0;i<4;++i)result.absolute[i]=double(child[i]);return result;
}
std::optional<std::array<int,4>> resize_child(std::array<int,2>parent,const Geometry&geometry,int mode,bool context_present,float view_scale,float context_d4){
 if(mode<0||mode>3)throw std::invalid_argument("unsupported child resize mode");if(mode==0)return std::nullopt;
 for(int v:parent)if(v<0||v>1000000)throw std::invalid_argument("resize parent bounds");
 auto finite=[](double value){return std::isfinite(value)&&std::abs(value)<=1000000.;};
 if(!finite(view_scale)||!finite(context_d4)||view_scale<0||context_d4<0)throw std::invalid_argument("resize scale domain");
 for(double v:geometry.relative)if(!finite(v))throw std::invalid_argument("relative geometry domain");for(double v:geometry.absolute)if(!finite(v))throw std::invalid_argument("absolute geometry domain");
 std::array<int,4>result;for(int i=0;i<4;++i){double value;
  if(mode==1)value=double(parent[i%2])*geometry.relative[i]+.5;
  else if(mode==2)value=double(context_present?view_scale:1.f)*geometry.absolute[i]+.5;
  else value=(double(context_present?context_d4:1.f)*geometry.absolute[i]+1.)*.5;
  if(!std::isfinite(value)||value<=-2147483649.||value>=2147483648.)throw std::invalid_argument("unrepresentable child bounds");result[i]=static_cast<std::int32_t>(value);
 }return result;
}
Geometry update_runtime_geometry(std::array<float,2>background,float view_scale,std::array<int,4>bounds){
 if(!std::isfinite(view_scale)||view_scale<=0)throw std::invalid_argument("runtime view scale");for(float d:background)if(!std::isfinite(d)||d<=0)throw std::invalid_argument("runtime background dimensions");
 Geometry result;const float inverse=1.f/view_scale;if(!std::isfinite(inverse))throw std::invalid_argument("runtime inverse scale");
 for(int i=0;i<4;++i){const float relative=float(bounds[i])/background[i%2],absolute=inverse*float(bounds[i]);if(!std::isfinite(relative)||!std::isfinite(absolute))throw std::invalid_argument("runtime geometry range");result.relative[i]=double(relative);result.absolute[i]=double(absolute);}return result;
}
float native_round(float value){auto bits=std::bit_cast<std::uint32_t>(value);const auto absolute=bits&0x7fffffffu;if(absolute>=0x7f800000u)throw std::invalid_argument("nonfinite round input");if(absolute>=0x4b000000u)return value;if(absolute<0x3f800000u)return std::bit_cast<float>((bits&0x80000000u)|(absolute>=0x3f000000u?0x3f800000u:0u));const auto shift=150u-(absolute>>23);const auto carry=(bits>>(shift-1u))&1u;return std::bit_cast<float>(((bits>>shift)+carry)<<shift);}
float native_trunc(float value){auto bits=std::bit_cast<std::uint32_t>(value);const auto absolute=bits&0x7fffffffu;if(absolute>=0x7f800000u)throw std::invalid_argument("nonfinite trunc input");if(absolute<0x3f800000u)return std::bit_cast<float>(bits&0x80000000u);if(absolute>=0x4b000000u)return value;const auto shift=150u-(absolute>>23);return std::bit_cast<float>(bits&~((1u<<shift)-1u));}
static void refresh_fixed_lengths(const RuntimeLayout&a,std::vector<RuntimeLayoutItem>&items){const int main=a.horizontal?0:1;for(auto&i:items)if(i.fixed)i.cached_length=(i.opaque_flag_a||i.fixed_current)?float(i.bounds[main+2]):i.view_scale*float(i.reference_bounds[main+2]);}
static std::vector<Geometry> update_layout_internal(const RuntimeLayout&a,std::vector<RuntimeLayoutItem>&items,bool parent_child_events){
 if(items.size()>256)throw std::invalid_argument("runtime layout item limit");for(int d:a.parent_dimensions)if(d<0)throw std::invalid_argument("negative parent size");for(float d:a.background_dimensions)if(!std::isfinite(d)||d<=0)throw std::invalid_argument("runtime background dimensions");if(!std::isfinite(a.view_scale)||a.view_scale<=0)throw std::invalid_argument("runtime view scale");
 for(const auto&i:items){if(i.minimum<0||i.maximum<i.minimum||!std::isfinite(i.cached_length)||i.cached_length<0||!std::isfinite(i.view_scale)||i.view_scale<=0||i.bounds[2]<0||i.bounds[3]<0||i.reference_bounds[2]<0||i.reference_bounds[3]<0)throw std::invalid_argument("runtime item domain");}
 const int main=a.horizontal?0:1,cross=1-main,cross_size=a.parent_dimensions[cross];float excess=0,flexible=0;
 for(const auto&i:items)if(i.visible)excess=excess+i.cached_length;excess=excess-float(a.parent_dimensions[main]);
 auto fixed_length=[&](const RuntimeLayoutItem&i){return i.fixed_current?float(i.bounds[main+2]):i.view_scale*float(i.reference_bounds[main+2]);};
 for(auto&i:items){
  if(i.keep_ratio){float next=i.cached_length;const int old_cross=i.bounds[cross+2];if(old_cross!=cross_size){if(cross_size==0)next=next/float(old_cross);else{next=float(cross_size)*next;if(old_cross!=0)next=next/float(old_cross);}}
   if(!std::isfinite(next))throw std::invalid_argument("runtime ratio range");if(i.visible)excess=excess+(next-i.cached_length);i.cached_length=next;
  }else if(i.fixed)i.cached_length=fixed_length(i);else if(i.visible)flexible=flexible+i.cached_length;
 }
 float used=flexible;if(excess<=flexible)used=excess;float available=flexible-used;if(available<=0)available=0;
 for(auto&i:items)if(!i.fixed&&!i.keep_ratio){if(flexible>=1){if(available>=1){i.cached_length=(i.cached_length*available)*(1.f/flexible);}else i.cached_length=(1.f/flexible)*i.cached_length;}else if(available>=1)i.cached_length=available*i.cached_length;}
 float actual_sum=0,ideal_sum=0,error=0;std::vector<Geometry>geometries;geometries.reserve(items.size());
 auto integer=[](float v){if(!std::isfinite(v)||double(v)<=-2147483649.||double(v)>=2147483648.)throw std::invalid_argument("unrepresentable runtime bound");return static_cast<std::int32_t>(v);};
 for(auto&i:items){float length=i.cached_length;
  if(i.fixed){length=fixed_length(i);i.cached_length=length;}
  else if(!i.keep_ratio){float rounded=native_round(length+error);const float lower=i.view_scale*float(i.minimum),upper=i.view_scale*float(i.maximum);if(rounded<lower)rounded=lower;if(rounded>=upper)rounded=upper;length=native_trunc(rounded);if(a.owner_cc_d0[0]==0&&a.owner_cc_d0[1]==0)i.cached_length=length;}
  std::array<int,4>bounds{};bounds[main]=integer(actual_sum);bounds[cross]=0;bounds[main+2]=integer(length);bounds[cross+2]=(parent_child_events||a.fill_cross)?cross_size:i.bounds[cross+2];const bool size_changed=i.bounds[2]!=bounds[2]||i.bounds[3]!=bounds[3];geometries.push_back(update_runtime_geometry(a.background_dimensions,a.view_scale,bounds));i.bounds=bounds;if(parent_child_events&&size_changed)refresh_fixed_lengths(a,items);
  if(i.visible){ideal_sum=ideal_sum+i.cached_length;actual_sum=actual_sum+float(i.bounds[main+2]);error=ideal_sum-actual_sum;}
 }
 return geometries;
}
std::vector<Geometry> update_layout(const RuntimeLayout&a,std::vector<RuntimeLayoutItem>&items){return update_layout_internal(a,items,false);}
void parent_resized(RuntimeLayoutState&state){
 if(state.geometry.size()!=state.items.size())throw std::invalid_argument("uninitialized runtime geometry");
 auto event=[&](auto&&self,bool resizing_owner)->void{
  if(!state.enabled)return;auto&a=state.layout;const int main=a.horizontal?0:1,limit=1-main;
  refresh_fixed_lengths(a,state.items);
  state.geometry=update_layout_internal(a,state.items,true);
  if(a.owner_cc_d0[limit]>=0){a.owner_cc_d0[limit]=a.parent_dimensions[main];return;}
  if(!resizing_owner){float sum=0;for(const auto&i:state.items)if(i.visible)sum=sum+i.cached_length;if(!std::isfinite(sum)||sum<1||double(sum)>=2147483648.)throw std::invalid_argument("unsupported autosized owner range");const int size=static_cast<std::int32_t>(sum);if(size!=a.parent_dimensions[main]){a.parent_dimensions[main]=size;a.background_dimensions={float(a.parent_dimensions[0]),float(a.parent_dimensions[1])};self(self,true);}}
 };
 event(event,false);
}
}
