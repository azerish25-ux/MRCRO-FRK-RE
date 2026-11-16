#pragma once
#include "mfrk/ui_parameter_controller.hpp"
namespace mfrk::ui {
// Actual parameter IDs/feature values, association policy and invalidation
// services are application-owned. These are source boundaries, not defaults.
struct GuiNotificationBoundary {
 virtual ~GuiNotificationBoundary()=default;
 virtual std::uint32_t parameter_id(GuiParameterValue&)=0;
 virtual GuiParameterValue& feature_parameter(GuiParameterBackend&,std::uint32_t)=0;
 virtual std::optional<std::uint32_t> mapped_id(std::uint32_t)=0;
 virtual bool mappable(std::uint32_t)=0;
 virtual std::uint32_t active_id()=0;
 virtual bool assignment_allowed(std::uint32_t)=0;
 virtual std::uint32_t context_selected_id()=0;
 virtual void invalidate(std::uintptr_t view)=0;
};
// Source6616b0 plus its primary changed/scalar deliveries661640/661660.
// Live descriptor, bound observer where control-info/alias paths require it,
// finite feature-state scalars, valid8/24-byte messages and stable service/row
// lifetimes are required. State remains live across callbacks. Diagnostics,
// concrete association policies and GUI invalidation bodies are separate owners.
class GuiParameterNotification {
 GuiParameterState& state_;GuiNotificationBoundary& boundary_;
 static std::uint32_t float_id(float value){
  if(!std::isfinite(value)||value<-9223372036854775808.f||value>=9223372036854775808.f)return 0;
  return static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
 }
 static std::uint32_t word(const preset_bank::Value& value,std::size_t offset,std::size_t size){
  if(value.bytes().size()!=size)throw std::invalid_argument("GUI notification data size");
  std::uint32_t result;std::memcpy(&result,value.bytes().data()+offset,4);return result;
 }
 void redraw(GuiControlInfo& info){if(info.view)boundary_.invalidate(info.view);}
 bool feature(std::uint32_t category,std::uint32_t event){const auto id=state_.backend->selection(category,0);return id&&*id==event;}
public:
 GuiParameterNotification(GuiParameterState& state,GuiNotificationBoundary& boundary):state_(state),boundary_(boundary){}
 void changed(){if(auto* observer=state_.observer)observer->changed(state_.id);}
 void scalar(const preset_bank::Value& value){if(auto* observer=state_.observer;observer&&value.tag()==1)observer->scalar_changed(state_.id,std::bit_cast<float>(value.scalar_bits()));}
 void notify(std::uint32_t event,preset_bank::Value& value,std::uint32_t origin){
  if(!state_.descriptor)throw std::logic_error("GUI descriptor required");
  std::optional<std::uint32_t> mapped;
  if(state_.descriptor->flags20&1)mapped=boundary_.mapped_id(state_.id);
  if(state_.control_info&&origin!=6){
   if(!state_.observer)throw std::logic_error("GUI notification observer required");
   auto* learn=state_.backend->mode_parameter(11,0);
   auto* hover=state_.backend->mode_parameter(14,0);
   auto* macro=state_.backend->mode_parameter(15,0);
   auto* assignment=state_.backend->mode_parameter(42,0);
   if(feature(2,event)){
    auto& info=*state_.control_info;const float f=gui_parameter_scalar(&value);
    if(info.flag08){info.flag21=f>0.f;redraw(info);}
   }else if(feature(3,event)){
    auto& target=boundary_.feature_parameter(*state_.backend,3);
    const auto* current=target.current();const auto selected=float_id(gui_parameter_scalar(current));
    if(!learn||gui_parameter_scalar(learn->current())<=0.f){
     auto& info=*state_.control_info;
     if(info.flag08||info.flag09){info.flag20=selected==state_.id;redraw(info);}
    }else{
     const bool can_map=boundary_.mappable(state_.id);
     const bool allowed=!can_map||boundary_.active_id()==state_.id;
     auto& info=*state_.control_info;
     if(info.flag08||info.flag09){info.flag20=mapped&&selected==*mapped&&allowed;redraw(info);}
    }
   }else if(feature(4,event)){
    const auto target=word(value,0,24);
    const auto* linked=state_.observer->linked_parameter;
    if(target==0xffffffffu||target==state_.id||(linked&&*linked==target)||(mapped&&*mapped==target)){
     auto& info=*state_.control_info;
     if(info.flag08){info.midi28=std::bit_cast<std::int32_t>(word(value,4,24));info.midi2c=std::bit_cast<std::int32_t>(word(value,8,24));redraw(info);}
     state_.observer->scalar_changed(state_.id,-1.f);
    }
   }else if(macro&&event==boundary_.parameter_id(*macro)){
    const auto target=word(value,0,8);auto& info=*state_.control_info;
    if(target==state_.id){if(info.flag0a){info.flag0c=word(value,4,8)!=0;redraw(info);}}
    else if(target==0xffffffffu&&info.flag0a){info.flag0c=false;redraw(info);}
   }else if(hover&&event==boundary_.parameter_id(*hover)){
    const auto target=float_id(gui_parameter_scalar(&value));auto& info=*state_.control_info;
    if(info.flag0a){info.flag0b=target==state_.id;redraw(info);}
   }else if(learn&&event==boundary_.parameter_id(*learn)){
    const bool permitted=mapped&&boundary_.assignment_allowed(state_.id);
    auto* info=state_.control_info;float f=gui_parameter_scalar(&value);
    if(info->flag09||info->flag0a){info->flag22=!(f<=0.f||permitted);redraw(*info);}
    info=state_.control_info;f=gui_parameter_scalar(&value);
    if(info->flag0a){info->flag23=f>0.f;redraw(*info);}
   }else if(assignment&&event==boundary_.parameter_id(*assignment)){
    auto* info=state_.control_info;const float f=gui_parameter_scalar(&value);
    if(info->flag0d){info->flag24=f>0.f;redraw(*info);}
    info=state_.control_info;const auto selected=boundary_.context_selected_id();
    if(info->flag0d){info->flag0e=state_.id==selected;redraw(*info);}
   }
  }
  if(origin==5){
   if(!state_.observer)throw std::logic_error("GUI alias observer required");
   const auto* linked=state_.observer->linked_parameter;
   if(linked&&*linked==event){
    for(std::size_t i=0;i<state_.observer->aliases.size();++i){
     if(state_.observer->aliases[i].enabled){
      const auto id=state_.observer->aliases[i].id;auto* backend=state_.backend;
      auto copy=value;backend->write(2,id,copy);
     }
    }
   }
  }
  if(origin!=state_.origin&&event==state_.id){if(origin==6)scalar(value);else changed();}
 }
};
}
