#pragma once
#include "mfrk/ui_write_owner.hpp"
#include <memory>
namespace mfrk::ui {
struct GuiUiMessage {virtual ~GuiUiMessage()=default;};
// The native controller-message family carries kind/origin/id/owned Value.
// Write messages use kind0; other valid kinds retain their actual app handlers.
struct GuiControllerMessage final:GuiUiMessage {
 std::uint32_t kind;
 GuiWriteMessage payload;
 GuiControllerMessage(std::uint32_t type,GuiWriteMessage&& value):kind(type),payload(std::move(value)){}
};
struct GuiMessageListener {
 virtual ~GuiMessageListener()=default;
 virtual void received(GuiUiMessage&)=0;
};
struct GuiControllerRecipient {
 virtual ~GuiControllerRecipient()=default;
 virtual void write(std::uint32_t origin,std::uint32_t id,preset_bank::Value&)=0;
 virtual void other_message(std::uint32_t kind,std::uint32_t origin,std::uint32_t id,preset_bank::Value&)=0;
};
struct GuiOtherControllerMessages {
 virtual ~GuiOtherControllerMessages()=default;
 // Actual handlers for667b10 kinds1..6, including preset operations. No default
 // success/no-op handlers are installed by the write-owner bridge.
 virtual void receive(std::uint32_t kind,std::uint32_t origin,std::uint32_t id,preset_bank::Value&)=0;
};
class GuiOwnedControllerRecipient final:public GuiControllerRecipient {
 GuiHardwareWriteState& state_;GuiWriteLock& lock_;
 GuiHardwareWriteBoundary& hardware_;GuiWriteDispatchBoundary& router_;
 GuiOtherControllerMessages& others_;
public:
 // Services stay alive through the recipient and all active callbacks.
 GuiOwnedControllerRecipient(GuiHardwareWriteState& state,GuiWriteLock& lock,GuiHardwareWriteBoundary& hardware,GuiWriteDispatchBoundary& router,GuiOtherControllerMessages& others)
 :state_(state),lock_(lock),hardware_(hardware),router_(router),others_(others){}
 void write(std::uint32_t origin,std::uint32_t id,preset_bank::Value& value)override{gui_write_owned(state_,lock_,hardware_,router_,origin,id,value);}
 void other_message(std::uint32_t kind,std::uint32_t origin,std::uint32_t id,preset_bank::Value& value)override{others_.receive(kind,origin,id,value);}
};
// Source667b10: test owner BEFORE the message-family cast. Type0 reenters the
// actual outer backend writer, never bypassing817fb0 to call678050 directly.
class GuiControllerMessageListener final:public GuiMessageListener {
 std::weak_ptr<GuiControllerRecipient> owner_;
public:
 explicit GuiControllerMessageListener(std::weak_ptr<GuiControllerRecipient> owner):owner_(std::move(owner)){}
 void received(GuiUiMessage& message)override{
  const auto owner=owner_.lock();if(!owner)return;
  const auto typed=dynamic_cast<GuiControllerMessage*>(&message);if(!typed)return;
  auto& value=typed->payload;
  if(typed->kind==0)owner->write(value.origin,value.id,value.value);
  else if(typed->kind<=6)owner->other_message(typed->kind,value.origin,value.id,value.value);
 }
};
// Sourcef7b750 weak listener lookup. The platform queue owns each envelope;
// delivery consumes its message exactly once on the queue's consumer thread.
// Strong holds keep valid callback objects alive for the duration of dispatch.
// This does not force a thread: the backend's actual router retains its own
// thread check and can requeue if delivery occurs on another thread.
class GuiMessageEnvelope {
 std::weak_ptr<GuiMessageListener> listener_;
 std::unique_ptr<GuiUiMessage> message_;
public:
 GuiMessageEnvelope(std::weak_ptr<GuiMessageListener> listener,std::unique_ptr<GuiUiMessage> message)
 :listener_(std::move(listener)),message_(std::move(message)){if(!message_)throw std::invalid_argument("Owned UI message required");}
 GuiMessageEnvelope(const GuiMessageEnvelope&)=delete;
 GuiMessageEnvelope& operator=(const GuiMessageEnvelope&)=delete;
 void deliver(){
  auto message=std::move(message_);if(!message)throw std::logic_error("UI message already delivered");
  if(const auto listener=listener_.lock())listener->received(*message);
 }
};
}
