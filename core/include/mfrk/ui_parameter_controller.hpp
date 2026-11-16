#pragma once
#include "mfrk/preset_bank.hpp"
#include <bit>
#include <cstdint>
namespace mfrk::ui {
struct GuiParameterValue {
 virtual ~GuiParameterValue()=default;
 // Source ParamInterface slot8. Returned storage belongs to the parameter.
 virtual const preset_bank::Value* current()=0;
};
inline float gui_parameter_scalar(const preset_bank::Value* value){
 return value&&value->tag()==1?std::bit_cast<float>(value->scalar_bits()):0.f;
}
struct GuiParameterOwner {
 virtual ~GuiParameterOwner()=default;
 virtual bool selection_capability()=0;
};
// JucedGuiParamController's product-backend interface. The application supplies
// the real backend; these calls do not replace it with a local scalar store.
struct GuiParameterBackend {
 virtual ~GuiParameterBackend()=default;
 virtual GuiParameterValue& parameter(std::uint32_t id)=0;
 virtual GuiParameterValue* mode_parameter(std::uint32_t id,std::uint32_t flags)=0;
 virtual void select(std::uint32_t id,bool primary,bool secondary,std::uintptr_t context)=0;
 virtual GuiParameterOwner& owner()=0;
 virtual std::optional<std::uint32_t> selection(std::uint32_t category,std::uint32_t flags)=0;
 virtual void begin(std::uint32_t origin,std::uint32_t id)=0;
 virtual void end(std::uint32_t origin,std::uint32_t id)=0;
 virtual void write(std::uint32_t origin,std::uint32_t id,preset_bank::Value&)=0;
 virtual void changed(std::uint32_t id,std::uintptr_t context)=0;
 virtual void prepared(std::uint32_t id,std::uintptr_t context)=0;
 virtual void ended(std::uint32_t id,std::uintptr_t context)=0;
};
struct GuiParameterObserver {
 // Source observer+40 points to a linked controller whose ID is at+8.
 // Link lifetime is owned by the application, not this callback interface.
 const std::uint32_t* linked_parameter=nullptr;
 virtual ~GuiParameterObserver()=default;
 virtual void changed(std::uint32_t id)=0;
};
struct GuiParameterDescriptor {
 std::uint32_t flags20=0;
 std::uint64_t count48=0;
 std::uint32_t flagsd0=0,flags204=0;
};
// Corresponds to root+50/+58/+5c/+60/+80. Registration allocates origin;
// it is not a fixed MIDI/GUI token. Callbacks may update these live fields.
struct GuiParameterState {
 GuiParameterBackend* backend;
 std::uint32_t id,origin;
 std::uintptr_t context;
 GuiParameterObserver* observer;
 const GuiParameterDescriptor* descriptor=nullptr;
};
// Source 6620e0/662900/662940/662980, diagnostics-disabled profile. Live, nonnull
// backend, stable controller lifetime and serial callbacks required. Logging,
// registration/unregistration, application selection policies and backend bodies are
// separate owners. In particular, this is not the complete GUI host factory.
class GuiParameterController {
 GuiParameterState& state_;
public:
 explicit GuiParameterController(GuiParameterState& state):state_(state){}
 // Source6620e0/64b7e0: resolve once, read stored raw scalar bits. A null or
 // data ParamValue returns +0 with diagnostics disabled. The receiver's
 // separately clamped normalized cache is deliberately not consulted.
 float current(){
  return gui_parameter_scalar(state_.backend->parameter(state_.id).current());
 }
 // Source662ce0. A live observer is required, as in the original method.
 bool edit(bool primary,bool secondary){
  auto* parameter=state_.backend->mode_parameter(11,0);
  const bool mode=parameter&&gui_parameter_scalar(parameter->current())>0.f;
  if(!state_.observer)throw std::logic_error("Edit observer required");
  const auto* linked=state_.observer->linked_parameter;
  if(linked&&mode)return false;
  state_.backend->select(linked?*linked:state_.id,primary,secondary,state_.context);
  auto& owner=state_.backend->owner();
  if(owner.selection_capability()){
   const auto selected=state_.backend->selection(2,0);
   return !selected||state_.id!=*selected;
  }
  const auto* descriptor=state_.descriptor;
  return descriptor&&(descriptor->flagsd0&1)!=0&&descriptor->count48<2&&mode;
 }
 bool prepare(){
  state_.backend->begin(state_.origin,state_.id);
  state_.backend->prepared(state_.id,state_.context);
  return true;
 }
 bool end(){
  state_.backend->end(state_.origin,state_.id);
  state_.backend->ended(state_.id,state_.context);
  return true;
 }
 void set(float incoming,bool notify){
  {
   auto value=preset_bank::Value::scalar(std::bit_cast<std::uint32_t>(incoming));
   state_.backend->write(state_.origin,state_.id,value);
  } // The backend may consume/replace value; destruction precedes notifications.
  if(notify){
   state_.backend->changed(state_.id,state_.context);
   if(auto* observer=state_.observer)observer->changed(state_.id);
  }
 }
};
}
