#pragma once
#include "mfrk/ui_component_repaint.hpp"
#include <vector>
namespace mfrk::ui {
struct GuiComponentBoundsProvider {
 virtual ~GuiComponentBoundsProvider()=default;
 virtual GuiDirtyBounds bounds()=0;
};
// Actual cutoff knob v90=b70010, v198=5a0600 (empty final override).
// Borrows live source fractional bounds (component+778..790) and the optional
// registered bounds-provider list. Providers and their list may change during
// callbacks; owners retain every referenced object. Finite double products,
// signed32-representable truncations/translations/unions are required.
class GuiKnobVisibleBounds {
 const std::array<double,4>& fractions_;
 const std::vector<GuiComponentBoundsProvider*>* providers_;
 static GuiDirtyBounds unite(GuiDirtyBounds first,const GuiDirtyBounds& next){
  if(next[2]<=0||next[3]<=0)return first;
  if(first[2]<=0||first[3]<=0)return next;
  const auto x=std::min(first[0],next[0]),y=std::min(first[1],next[1]);
  const auto right=std::max(first[0]+first[2],next[0]+next[2]);
  const auto bottom=std::max(first[1]+first[3],next[1]+next[3]);
  return {x,y,right-x,bottom-y};
 }
public:
 GuiKnobVisibleBounds(const std::array<double,4>& fractions,
  const std::vector<GuiComponentBoundsProvider*>* providers)
  :fractions_(fractions),providers_(providers){}
 GuiDirtyBounds get(const GuiDirtyBounds& component){
  auto result=component;
  if(fractions_[2]>0.&&fractions_[3]>0.){
   result={static_cast<std::int32_t>(static_cast<double>(component[2])*fractions_[0])+component[0],
           static_cast<std::int32_t>(static_cast<double>(component[3])*fractions_[1])+component[1],
           static_cast<std::int32_t>(fractions_[2]*static_cast<double>(component[2])),
           static_cast<std::int32_t>(fractions_[3]*static_cast<double>(component[3]))};
  }
  if(providers_&&!providers_->empty()){
   auto accumulated=(*providers_)[0]->bounds();
   for(std::size_t i=1;i<providers_->size();++i)accumulated=unite(accumulated,(*providers_)[i]->bounds());
   result=unite(result,accumulated);
  }
  // The observed knob's final v198 is the actual zero-rectangle method.
  return result;
 }
};
}
