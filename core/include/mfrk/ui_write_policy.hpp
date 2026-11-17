#pragma once
#include "mfrk/ui_parameter_controller.hpp"
#include "mfrk/parameter_receiver.hpp"
namespace mfrk::ui {
struct GuiWriteDescriptor:GuiParameterDescriptor {
 std::uint32_t scope50=0xffffffffu,group200=0;
 std::uint64_t sizec0=4;
 bool flag250=false;
};
struct GuiWritePolicyState {
 std::uint32_t mode11c30=0,instrument11ab8=0,instrumentdc78=0;
 std::uint32_t automationdad8=0xffffffffu,performeddad4=0xffffffffu;
 bool enabledcca8=false,suppressedcca9=false,flag12060=false,flagdfe7=false;
 bool flage010=false,flage118=false,flage220=false;
};
struct GuiWriteReceiver {
 virtual ~GuiWriteReceiver()=default;
 virtual preset_bank::Value& stored()=0;
 virtual bool equal(const preset_bank::Value&)=0;
 virtual void set(preset_bank::Value&,std::uint32_t origin,std::uint8_t mode)=0;
 virtual void direct(preset_bank::Value&)=0;
};
// Source64b710 for the admitted Value1/2/3 representation. Scalar comparison
// retains the native unordered-NaN behavior; data uses size then byte equality.
inline bool gui_write_values_equal(const preset_bank::Value& a,const preset_bank::Value& b){
 if((a.tag()==1)!=(b.tag()==1))return false;
 if(a.tag()==1)return parameter_value_bits::equal_scalar(a.scalar_bits(),b.scalar_bits());
 return a.bytes().size()==b.bytes().size()&&std::equal(a.bytes().begin(),a.bytes().end(),b.bytes().begin());
}
// Binds the policy to actual owned internal parameter storage and notification
// services. Source InternalParam vtable181be22c0.v48=6d1ac0 broadcasts incoming
// with origin6 without changing storage/dirty state or consuming that value.
template<class Boundary>
class GuiInternalWriteReceiver final:public GuiWriteReceiver {
 preset_bank::InternalReceiver& receiver_;Boundary& boundary_;
public:
 GuiInternalWriteReceiver(preset_bank::InternalReceiver& receiver,Boundary& boundary):receiver_(receiver),boundary_(boundary){}
 preset_bank::Value& stored()override{return receiver_.value;}
 bool equal(const preset_bank::Value& value)override{return gui_write_values_equal(value,receiver_.value);}
 void set(preset_bank::Value& value,std::uint32_t origin,std::uint8_t mode)override{parameter_receiver::set(receiver_,std::move(value),origin,mode,boundary_);}
 void direct(preset_bank::Value& value)override{boundary_.notify(receiver_.id,value,6);}
};
// Source VstParam vtable181bc6c08 shares the direct broadcast method. Ordinary
// writes also run its recovered cache/FObject update through set_current.
template<class Boundary>
class GuiPositiveWriteReceiver final:public GuiWriteReceiver {
 preset_bank::Receiver& receiver_;Boundary& boundary_;
public:
 GuiPositiveWriteReceiver(preset_bank::Receiver& receiver,Boundary& boundary):receiver_(receiver),boundary_(boundary){}
 // The receiver itself is mutable. This borrows its real stored value so a
 // callback receives the same storage as the native ParamInterface getter.
 preset_bank::Value& stored()override{return const_cast<preset_bank::Value&>(receiver_.value());}
 bool equal(const preset_bank::Value& value)override{return gui_write_values_equal(value,receiver_.value());}
 void set(preset_bank::Value& value,std::uint32_t origin,std::uint8_t mode)override{
  if(!receiver_.set_current(std::move(value),origin,mode,boundary_).has_value())throw std::invalid_argument("Positive write requires scalar input");
 }
 void direct(preset_bank::Value& value)override{boundary_.notify(receiver_.id(),value,6);}
};
struct GuiWriteTransaction {
 virtual ~GuiWriteTransaction()=default;
 virtual bool busy()=0;
 virtual void prepare(std::uint32_t id,std::uint32_t part,std::uint32_t group)=0;
};
struct GuiWriteReplacement {
 std::array<std::optional<std::uint32_t>,3> ids;
 std::array<std::uint32_t,3> scalar_bits;
};
// These are actual application owners/services, not success/default observers.
// Native comparisons may observe their boundaries separately; the scene host
// must provide concrete implementations before installing a writable control.
struct GuiWritePolicyBoundary {
 virtual ~GuiWritePolicyBoundary()=default;
 virtual void begin(std::uint32_t)=0;
 virtual void end(std::uint32_t)=0;
 virtual void prewrite(std::uint32_t origin,std::uint32_t id,preset_bank::Value&)=0;
 virtual GuiWriteDescriptor& descriptor(std::uint32_t)=0;
 virtual void special(std::uint32_t id,float,std::uint32_t instrument)=0;
 virtual GuiWriteTransaction* transaction()=0;
 virtual bool skip_transaction()=0;
 virtual std::vector<std::pair<std::uint32_t,float>> transforms(std::uint8_t part,std::uint32_t id,float)=0;
 virtual void backend_write(std::uint32_t origin,std::uint32_t id,preset_bank::Value&)=0;
 virtual void changed(std::uint32_t id,float)=0;
 virtual bool feature_enabled(std::uint32_t)=0;
 virtual std::optional<std::uint32_t> feature(std::uint32_t)=0;
 virtual std::optional<std::uint32_t> reset_id(std::uint32_t instrument)=0;
 virtual void reset_note(std::uint32_t)=0;
 virtual void association_single(std::uint32_t origin)=0;
 virtual void association_many()=0;
 virtual void association_parameter(std::uint32_t origin,std::uint32_t id)=0;
 virtual std::optional<std::uint32_t> associated_id(const GuiWriteDescriptor&)=0;
 virtual void associated_value(std::uint32_t origin,std::uint32_t id,preset_bank::Value&)=0;
 virtual void association_changed(std::uint32_t id)=0;
 // Resolves the current selector context separately for each call, as native.
 virtual std::optional<std::uint32_t> binding(unsigned group,unsigned index)=0;
 virtual void save_part(std::uint32_t part,std::uint32_t id)=0;
 virtual bool associated_rows_empty(std::uint32_t id)=0;
 virtual bool part_context_allowed(std::uint32_t part)=0;
 virtual const GuiWriteReplacement* replacement(std::uint32_t part)=0;
 virtual GuiWriteDescriptor& host_descriptor(std::uint32_t id)=0;
 virtual void host_edit(std::uint32_t id,double value)=0;
 virtual void processor(std::uint32_t id,preset_bank::Value&)=0;
};
// Full678190 control flow, diagnostic channels disabled. Real descriptor,
// receiver, transaction, selector, association and host services are required.
// Successful allocations, valid Value tags and serial, nonthrowing callbacks;
// referenced state and descriptors stay live and may change during callbacks.
inline bool gui_apply_write(GuiWritePolicyState& state,std::uint32_t origin,
 std::uint32_t id,preset_bank::Value& incoming,GuiWriteReceiver* receiver,
 std::uint32_t part,GuiWritePolicyBoundary& boundary){
 if(!receiver)return false;
 if(origin==10)boundary.begin(id);
 if(origin>8||((0x112u>>origin)&1u)==0)boundary.prewrite(origin,id,incoming);
 auto& descriptor=boundary.descriptor(id);
 if(((descriptor.typea8&0xffu)==1)!=(incoming.tag()==1)||
    (!(descriptor.flags204&(1u<<9))&&descriptor.sizec0<incoming.bytes().size()))return false;
 if(state.mode11c30==7&&(descriptor.flagsd0&0x8000u)&&origin==3){
  boundary.special(id,gui_parameter_scalar(&incoming),state.instrument11ab8);return true;
 }
 if(origin==0&&(id&0x80000000u)==0&&state.automationdad8==0xffffffffu)state.automationdad8=id;
 auto* transaction=boundary.transaction();
 if(transaction&&origin!=4&&origin!=1&&origin!=8&&!(origin==3&&!descriptor.flag250)){
  if((descriptor.flags204&7u)&&!(descriptor.flags204&8u)&&!transaction->busy()&&
     !boundary.skip_transaction()&&!state.flag12060&&!state.flagdfe7&&
     (!receiver->equal(incoming)||origin==0)){
   // A callback can replace the actual transaction owner.
   auto* current=boundary.transaction();
   if(!current)throw std::logic_error("Write transaction disappeared");
   current->prepare(id,part,descriptor.group200);
  }
 }
 if(origin==6)receiver->direct(incoming);
 else receiver->set(incoming,origin,origin==4||origin==1?0:1);
 auto& value=receiver->stored();
 if(!state.flage010){
  if(!state.flage118&&!state.flage220&&value.tag()==1){
   const auto transformed=boundary.transforms(static_cast<std::uint8_t>(part),id,gui_parameter_scalar(&value));
   for(const auto& [target,scalar]:transformed){auto copy=preset_bank::Value::scalar(std::bit_cast<std::uint32_t>(scalar));boundary.backend_write(origin,target,copy);}
  }else if(state.flage118||state.flage220)boundary.changed(id,gui_parameter_scalar(&value));
 }
 if(origin>11&&boundary.feature_enabled(5)){
  const auto selected=boundary.feature(5);
  if(selected&&*selected==id){
   if(const auto reset=boundary.reset_id(state.instrumentdc78)){
    auto one=preset_bank::Value::scalar(0x3f800000u);boundary.backend_write(2,*reset,one);
    auto zero=preset_bank::Value::scalar(0);boundary.backend_write(2,*reset,zero);
    for(std::uint32_t note=0;note<128;++note)boundary.reset_note(note);
   }
  }
 }
 if(descriptor.flags20&&origin!=1&&origin!=4){
  if(origin==6)return false;
  if(origin!=8&&origin!=9){
   if(descriptor.flags20&1u){if(descriptor.count48==1)boundary.association_single(origin);else boundary.association_many();}
   if(descriptor.flags20&0x198u)boundary.association_parameter(origin,id);
   if(descriptor.flags20&(1u<<5))if(const auto target=boundary.associated_id(descriptor))boundary.associated_value(origin,*target,value);
   if(descriptor.flags20&(1u<<6))if(const auto target=boundary.associated_id(descriptor)){auto copy=value;boundary.backend_write(origin,*target,copy);}
   if(descriptor.flags20&(1u<<9))boundary.association_changed(id);
  }
 }
 if(origin==6)return false;
 bool forced=origin>=9||origin==2;
 if(state.mode11c30==3){
  if(origin==5)forced=true;
  else if(origin==0&&!(descriptor.flagsd0&1u)){
   bool found=false;
   for(unsigned group=0;group<2&&!found;++group)for(unsigned index=0;index<17;++index){const auto target=boundary.binding(group,index);if(target&&*target==id){found=true;break;}}
   if(found)forced=true;
  }
 }
 const bool deliver=(!state.suppressedcca9&&forced&&state.enabledcca8)||(descriptor.flags204&(1u<<7));
 if(deliver||origin==3){
  if((descriptor.flags204&4u)&&state.enabledcca8)boundary.save_part(part,id);
  if(deliver){
   if((id&0x80000000u)||part){
    const bool allowed=!(origin>11&&part)||boundary.associated_rows_empty(id);
    if(allowed&&boundary.part_context_allowed(part)){
     std::optional<std::uint32_t> substitute;
     if(const auto* replacements=boundary.replacement(part))for(unsigned i=0;i<3;++i)if(replacements->ids[i]&&*replacements->ids[i]==id){substitute=replacements->scalar_bits[i];break;}
     if(substitute){auto temporary=preset_bank::Value::scalar(*substitute);boundary.processor(id,temporary);}
     else boundary.processor(id,value);
    }
   }else{
    const auto& host_descriptor=boundary.host_descriptor(id);
    const bool suppressed=(host_descriptor.flags20&1u)&&host_descriptor.scope50==0xffffffffu&&host_descriptor.count48==1;
    if(state.mode11c30==7&&state.automationdad8!=0xffffffffu&&id!=state.automationdad8)boundary.processor(id,value);
    else if(!suppressed){state.performeddad4=id;boundary.host_edit(id,static_cast<double>(gui_parameter_scalar(&value)));state.performeddad4=0xffffffffu;}
   }
  }
 }
 if(origin==10)boundary.end(id);
 if(origin==0&&state.automationdad8==id)state.automationdad8=0xffffffffu;
 return true;
}
}
