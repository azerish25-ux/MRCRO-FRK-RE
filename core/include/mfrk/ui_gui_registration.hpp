#pragma once
#include "mfrk/ui_parameter_notification.hpp"
#include "mfrk/parameter_notifier.hpp"
#include "mfrk/ui_gui_ids.hpp"
namespace mfrk::ui {
struct GuiRegistrationContext {
 virtual ~GuiRegistrationContext()=default;
 virtual std::uint32_t context(GuiParameterBackend&)=0;
 virtual void prepare_part(GuiParameterBackend&,std::uint32_t part,std::uint32_t context)=0;
 virtual bool receiver_exists(GuiParameterBackend&,std::uint32_t id,std::uint32_t context)=0;
};
struct GuiRegistrationBoundary {
 virtual ~GuiRegistrationBoundary()=default;
 virtual std::uint32_t register_controller(GuiParameterBackend&,std::uint32_t id,parameter_dispatch::ListenerRegistry&)=0;
 virtual void unregister_controller(GuiParameterBackend&,std::uint32_t id,parameter_dispatch::ListenerRegistry&)=0;
 virtual bool feature(GuiParameterBackend&,std::uint32_t kind,std::uint32_t part,parameter_dispatch::ListenerRegistry&,bool subscribe)=0;
 virtual bool property(GuiParameterBackend&,std::uint32_t kind,std::uint32_t part,parameter_dispatch::ListenerRegistry&,bool subscribe)=0;
};
// Concrete source registration bridge67aff0/67b860/67cc70/67ce30..67cf40.
// Counter and selector tables are supplied actual owner state, not guessed
// origins or parameter IDs. Part preparation/receiver resolution are real
// bank services; no successful-existence fallback is provided.
class GuiNotifierRegistration final:public GuiRegistrationBoundary {
 parameter_dispatch::ParameterNotifier& notifier_;
 parameter_dispatch::NotifierDirectory& directory_;
 const GuiIdTables& tables_;std::uint32_t& counter_;GuiRegistrationContext& context_;
 static std::uint32_t part(std::uint32_t id){return (id&0x7fffffffu)/1000000u;}
 bool watch(GuiParameterBackend& backend,std::optional<std::uint32_t> id,parameter_dispatch::ListenerRegistry& listener,bool subscribe){
  if(!id)return false;
  if(subscribe){const auto mode=context_.context(backend);context_.prepare_part(backend,part(*id),mode);if(!context_.receiver_exists(backend,*id,mode))throw std::out_of_range("GUI parameter receiver missing");notifier_.add(*id,&listener,mode!=0);}
  else if(const auto owner=listener.find(*id))directory_.remove(owner,*id,listener.identity());
  return true;
 }
public:
 GuiNotifierRegistration(parameter_dispatch::ParameterNotifier& notifier,parameter_dispatch::NotifierDirectory& directory,const GuiIdTables& tables,std::uint32_t& counter,GuiRegistrationContext& context)
 :notifier_(notifier),directory_(directory),tables_(tables),counter_(counter),context_(context){}
 std::uint32_t register_controller(GuiParameterBackend& backend,std::uint32_t id,parameter_dispatch::ListenerRegistry& listener)override{
  if(id!=0xffffffffu){context_.prepare_part(backend,part(id),0);notifier_.add(id,&listener,false);}return counter_++;
 }
 void unregister_controller(GuiParameterBackend& backend,std::uint32_t id,parameter_dispatch::ListenerRegistry& listener)override{
  if(id==0xffffffffu)return;
  const auto mode=context_.context(backend);
  if(context_.receiver_exists(backend,id,mode))if(const auto owner=listener.find(id))directory_.remove(owner,id,listener.identity());
 }
 bool feature(GuiParameterBackend& backend,std::uint32_t kind,std::uint32_t slot,parameter_dispatch::ListenerRegistry& listener,bool subscribe)override{return watch(backend,tables_.feature(context_.context(backend),kind,slot),listener,subscribe);}
 bool property(GuiParameterBackend& backend,std::uint32_t kind,std::uint32_t slot,parameter_dispatch::ListenerRegistry& listener,bool subscribe)override{return watch(backend,tables_.property(context_.context(backend),kind,slot),listener,subscribe);}
};
// Source6612b0/6614c0 feature subscription lifetime, with actual owned reverse
// registry, controller methods and primary notification handler. Application
// descriptor, backend/observer and service objects outlive this owner.
class GuiRegisteredParameter {
 GuiParameterState state_;
 parameter_dispatch::ListenerRegistry listener_;
 GuiParameterController controller_;
 GuiParameterNotification notification_;
 GuiRegistrationBoundary& registration_;
 bool closed_=false;
 void feature(std::uint32_t id,bool subscribe){registration_.feature(*state_.backend,id,0,listener_,subscribe);}
 void property(std::uint32_t id,bool subscribe){registration_.property(*state_.backend,id,0,listener_,subscribe);}
public:
 GuiRegisteredParameter(std::uint64_t identity,parameter_dispatch::NotifierDirectory& directory,GuiParameterBackend& backend,std::uint32_t id,std::uintptr_t context,GuiParameterObserver* observer,const GuiParameterDescriptor& descriptor,GuiNotificationBoundary& notification,GuiRegistrationBoundary& registration)
 :state_{&backend,id,0,context,nullptr},listener_(identity,directory),controller_(state_),notification_(state_,notification),registration_(registration){
  state_.origin=registration_.register_controller(backend,id,listener_);state_.observer=observer;state_.descriptor=&descriptor;
  const bool first=(descriptor.flags20&1)!=0;auto flags=descriptor.flags204;
  if((flags&(1u<<16))||first||((descriptor.flagsd0&1)&&descriptor.count48<2)){feature(3,true);flags=descriptor.flags204;}
  if(flags&(1u<<16)){feature(4,true);feature(2,true);}
  if(first||((state_.descriptor->flagsd0&1)&&state_.descriptor->count48<2)){property(11,true);property(14,true);property(15,true);}
  if(state_.descriptor->flags204&(1u<<18))property(42,true);
 }
 GuiRegisteredParameter(const GuiRegisteredParameter&)=delete;
 GuiRegisteredParameter& operator=(const GuiRegisteredParameter&)=delete;
 ~GuiRegisteredParameter(){close();}
 void close(){
  if(closed_)return;
  closed_=true;
  const auto* descriptor=state_.descriptor;const bool first=(descriptor->flags20&1)!=0;
  if((descriptor->flags204&(1u<<16))||first||((descriptor->flagsd0&1)&&descriptor->count48<2)){feature(3,false);descriptor=state_.descriptor;}
  if(descriptor->flags204&(1u<<16)){feature(4,false);feature(2,false);}
  if(first||((state_.descriptor->flagsd0&1)&&state_.descriptor->count48<2)){property(11,false);property(14,false);property(15,false);}
  if(state_.descriptor->flags204&(1u<<18))property(42,false);
  registration_.unregister_controller(*state_.backend,state_.id,listener_);listener_.close();
 }
 GuiParameterState& state()noexcept{return state_;}
 GuiParameterController& controller()noexcept{return controller_;}
 parameter_dispatch::ListenerRegistry& listener()noexcept{return listener_;}
 void notify(std::uint32_t id,preset_bank::Value& value,std::uint32_t origin){notification_.notify(id,value,origin);}
};
}
