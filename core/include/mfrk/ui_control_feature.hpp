#pragma once
#include "mfrk/ui_control_parameters.hpp"
#include <limits>
namespace mfrk::ui {
struct GuiSliderPresentation {
 bool reverse=false;
 float scalar_cache=-1.f,view_cache=0.f;
 std::uint32_t feature_flags=0;
 std::uint8_t component_flags=0;
};
struct GuiControlFeatureState {
 bool reverse=false,flag09=false,flag3c=false,active=true,disabled_data=false;
 std::uint32_t mean_count=0,format=0;
 std::uint64_t slice_offset=0,slice_length=~std::uint64_t(0),capacity=0,disabled_offset=0;
 std::int32_t value_index=-1,disabled_index=-1;
 std::array<float,5> statistics{}; // min,max,mean,min-position,max-position
 std::vector<float> history;
};
struct GuiControlFeatureBoundary {
 virtual ~GuiControlFeatureBoundary()=default;
 virtual std::uint32_t parameter_id(std::uint64_t index)=0;
 virtual float current(std::uint32_t id)=0;
 virtual bool is_data(std::uint32_t id)=0;
 virtual std::span<const std::uint8_t> data(std::uint32_t id)=0;
 virtual void scalar_changed(float previous,float current)=0;
 virtual void range_changed(const std::array<float,5>& previous,const std::array<float,5>& current)=0;
 virtual void invalidate()=0;
 virtual void set_component_state(bool active)=0;
};
// Source b9e7e0 packed eight-at-a-time accumulation and horizontal reduction.
// Strict FP/contraction disabled; the order is deliberately not std::accumulate.
inline float gui_feature_sum(std::span<const float> values){
 std::size_t index=0;float total=0.f;
 if(values.size()>=8){
  std::array<float,4> first{},second{};
  for(;index+8<=values.size();index+=8)for(unsigned j=0;j<4;++j){first[j]=first[j]+values[index+j];second[j]=second[j]+values[index+j+4];}
  for(unsigned j=0;j<4;++j)first[j]=first[j]+second[j];
  const float low=first[2]+first[0],high=first[3]+first[1];total=low+high;
 }
 for(;index<values.size();++index)total=total+values[index];
 return total;
}
// Source f55d60/f56450. Caller supplies actual feature configuration, live
// presentation and controller services. Finite values, valid byte ranges,
// history<=capacity, incoming sample count<=capacity and capacity<=INT_MAX.
// Widget-specific dirty-region callbacks remain actual scene implementations.
class GuiControlFeature {
 GuiControlFeatureState& state_;GuiSliderPresentation& view_;GuiControlFeatureBoundary& boundary_;
 void active(bool value){
  if(value==state_.active)return;
  state_.active=value;
  if(!value&&state_.capacity){state_.history.clear();state_.statistics[0]=-1.f;state_.statistics[1]=-1.f;state_.statistics[2]=-1.f;}
  if(state_.flag09)view_.feature_flags=value?2u:0u;
  boundary_.invalidate();
 }
 void make_room(std::size_t incoming){
  if(state_.capacity>static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())||state_.history.size()>state_.capacity||incoming>state_.capacity)throw std::invalid_argument("Feature history profile");
  const auto combined=state_.history.size()+incoming;
  if(combined>state_.capacity)state_.history.erase(state_.history.begin(),state_.history.begin()+static_cast<std::ptrdiff_t>(combined-state_.capacity));
 }
 void range(){
  std::array<float,5> next{};const auto count=state_.history.size();
  if(count){
   float minimum=1.f,maximum=0.f;std::size_t min_index=0,max_index=0;
   // Native statistics exclude sample0 from extrema and scan newest first.
   for(std::size_t index=count-1;index>0;--index){const float value=state_.history[index];if(value<minimum){minimum=value;min_index=index;}if(value>maximum){maximum=value;max_index=index;}}
   next[0]=minimum;next[1]=maximum;
   const auto padding=state_.capacity-count;
   const float inverse=1.f/static_cast<float>(state_.capacity);
   next[3]=static_cast<float>(padding+1+min_index)*inverse;
   next[4]=static_cast<float>(padding+1+max_index)*inverse;
   const auto averaged=state_.mean_count==0||count<state_.mean_count?count:static_cast<std::size_t>(state_.mean_count);
   next[2]=gui_feature_sum(std::span<const float>(state_.history).last(averaged))/static_cast<float>(averaged);
  }
  if(state_.reverse){next[0]=1.f-next[0];next[1]=1.f-next[1];next[2]=1.f-next[2];}
  bool changed=false;for(unsigned i=0;i<4;++i)changed|=std::bit_cast<std::uint32_t>(next[i])!=std::bit_cast<std::uint32_t>(state_.statistics[i]);
  changed|=!parameter_value_bits::equal_scalar(std::bit_cast<std::uint32_t>(next[4]),std::bit_cast<std::uint32_t>(state_.statistics[4]));
  if(changed){const auto previous=state_.statistics;state_.statistics=next;boundary_.range_changed(previous,next);}
 }
public:
 GuiControlFeature(GuiControlFeatureState& state,GuiSliderPresentation& view,GuiControlFeatureBoundary& boundary):state_(state),view_(view),boundary_(boundary){}
 void update(std::uint32_t event){
  if(state_.value_index!=-1&&event==boundary_.parameter_id(static_cast<std::uint64_t>(state_.value_index))){
   if(!state_.capacity){
    float value=boundary_.current(event);if(view_.reverse)value=1.f-value;if(state_.reverse)value=1.f-value;
    const float previous=state_.statistics[2];
    if(!parameter_value_bits::equal_scalar(std::bit_cast<std::uint32_t>(value),std::bit_cast<std::uint32_t>(previous))){boundary_.scalar_changed(previous,value);state_.statistics[2]=value;}
   }else if(state_.active){
    if(!boundary_.is_data(event)){
     float value=boundary_.current(event);if(view_.reverse)value=1.f-value;
     make_room(1);state_.history.push_back(value);
    }else{
     auto bytes=boundary_.data(event);
     if(state_.slice_offset<=bytes.size()&&state_.slice_length<=bytes.size()-state_.slice_offset)bytes=bytes.subspan(static_cast<std::size_t>(state_.slice_offset),static_cast<std::size_t>(state_.slice_length));
     if(state_.format==0){make_room(bytes.size()/4);for(std::size_t i=0;i+4<=bytes.size();i+=4){std::uint32_t word;std::memcpy(&word,bytes.data()+i,4);state_.history.push_back(std::bit_cast<float>(word));}}
     else{make_room(bytes.size());for(const auto byte:bytes)state_.history.push_back(static_cast<float>(byte)*std::bit_cast<float>(0x3b808081u));}
    }
    range();
   }
  }
  if(state_.disabled_index!=-1&&event==boundary_.parameter_id(static_cast<std::uint64_t>(state_.disabled_index))){
   bool enabled;
   if(state_.disabled_data){const auto bytes=boundary_.data(event);if(state_.disabled_offset>=bytes.size())throw std::out_of_range("Feature disabled byte");enabled=bytes[static_cast<std::size_t>(state_.disabled_offset)]!=0;}
   else{float value=boundary_.current(event);if(view_.reverse)value=1.f-value;enabled=value>.5f;}
   active(enabled);
   if(state_.flag3c&&enabled!=((view_.component_flags&2u)!=0))boundary_.set_component_state(enabled);
  }
 }
};
}
