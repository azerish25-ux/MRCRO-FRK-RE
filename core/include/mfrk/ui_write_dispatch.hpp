#pragma once
#include "mfrk/preset_bank.hpp"
namespace mfrk::ui {
struct GuiWriteMessage {
 std::uint32_t origin,id;
 preset_bank::Value value;
};
struct GuiWriteDispatchBoundary {
 virtual ~GuiWriteDispatchBoundary()=default;
 virtual std::optional<std::uint64_t> message_thread()=0;
 virtual std::uint64_t current_thread()=0;
 // Ownership transfers to the real message queue. Delivery must reenter the
 // backend on its message thread, not mutate a bank from the submitting thread.
 virtual void post(std::unique_ptr<GuiWriteMessage>)=0;
 virtual std::uint32_t part_count()=0;
 virtual void prepare_part(std::uint32_t)=0;
 // 6682f0 may return null. The actual write policy, not this router, handles it.
 virtual std::uintptr_t receiver(std::uint32_t id,std::uint32_t part)=0;
 virtual void apply(std::uint32_t origin,std::uint32_t id,preset_bank::Value&,
                    std::uintptr_t receiver,std::uint32_t part)=0;
};
enum class GuiWriteRoute {queued,invalid_part,applied};
// Source678050 plus64ea00, diagnostics disabled. The original AL after the
// synchronous policy call is not a defined success contract; this enum reports
// only the route taken. Queue allocation and posting must succeed. No fallback
// scalar store, synchronous queue emulation or receiver-success default exists.
inline GuiWriteRoute gui_dispatch_write(std::uint32_t origin,std::uint32_t id,
 preset_bank::Value& value,GuiWriteDispatchBoundary& boundary){
 const auto thread=boundary.message_thread();
 if(thread&&boundary.current_thread()!=*thread){
  auto message=std::make_unique<GuiWriteMessage>(GuiWriteMessage{origin,id,std::move(value)});
  boundary.post(std::move(message));
  return GuiWriteRoute::queued;
 }
 const auto part=(id&0x7fffffffu)/1000000u;
 if(part>=boundary.part_count())return GuiWriteRoute::invalid_part;
 boundary.prepare_part(part);
 const auto receiver=boundary.receiver(id,part);
 boundary.apply(origin,id,value,receiver,part);
 return GuiWriteRoute::applied;
}
}
