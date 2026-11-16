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
// JucedGuiParamController's product-backend interface. The application supplies
// the real backend; these calls do not replace it with a local scalar store.
struct GuiParameterBackend {
 virtual ~GuiParameterBackend()=default;
 virtual GuiParameterValue& parameter(std::uint32_t id)=0;
 virtual void begin(std::uint32_t origin,std::uint32_t id)=0;
 virtual void end(std::uint32_t origin,std::uint32_t id)=0;
 virtual void write(std::uint32_t origin,std::uint32_t id,preset_bank::Value&)=0;
 virtual void changed(std::uint32_t id,std::uintptr_t context)=0;
 virtual void prepared(std::uint32_t id,std::uintptr_t context)=0;
 virtual void ended(std::uint32_t id,std::uintptr_t context)=0;
};
struct GuiParameterObserver {
 virtual ~GuiParameterObserver()=default;
 virtual void changed(std::uint32_t id)=0;
};
// Corresponds to root+50/+58/+5c/+60/+80. Registration allocates origin;
// it is not a fixed MIDI/GUI token. Callbacks may update these live fields.
struct GuiParameterState {
 GuiParameterBackend* backend;
 std::uint32_t id,origin;
 std::uintptr_t context;
 GuiParameterObserver* observer;
};
// Source 6620e0/662900/662940/662980, diagnostics-disabled profile. Live, nonnull
// backend, stable controller lifetime and serial callbacks required. Logging,
// registration/unregistration, edit-mode selection and backend bodies are
// separate owners. In particular, this is not the complete GUI host factory.
class GuiParameterController {
 GuiParameterState& state_;
public:
 explicit GuiParameterController(GuiParameterState& state):state_(state){}
 // Source6620e0/64b7e0: resolve once, read stored raw scalar bits. A null or
 // data ParamValue returns +0 with diagnostics disabled. The receiver's
 // separately clamped normalized cache is deliberately not consulted.
 float current(){
  const auto* value=state_.backend->parameter(state_.id).current();
  return value&&value->tag()==1?std::bit_cast<float>(value->scalar_bits()):0.f;
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
