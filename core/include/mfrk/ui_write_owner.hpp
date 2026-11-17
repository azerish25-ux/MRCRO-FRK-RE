#pragma once
#include "mfrk/ui_write_dispatch.hpp"
#include "mfrk/ui_parameter_controller.hpp"
#include <cmath>
#include <limits>
#include <mutex>
namespace mfrk::ui {
struct GuiWriteLock {
 virtual ~GuiWriteLock()=default;
 // Busy returns false. Actual mutex errors propagate; they are not busy.
 virtual bool try_lock()=0;
 virtual void unlock()=0;
};
class GuiWriteMutex final:public GuiWriteLock {
 std::mutex mutex_;
public:
 bool try_lock()override{return mutex_.try_lock();}
 void unlock()override{mutex_.unlock();}
};
struct GuiHardwareWriteState {
 std::uint32_t special_id,mode;
 std::int32_t cached_integer;
 bool enabled,guard;
};
struct GuiHardwareWriteBoundary {
 virtual ~GuiHardwareWriteBoundary()=default;
 virtual bool eligible(std::uint32_t origin,std::uint32_t id,preset_bank::Value&)=0;
 // Own the actual send/status lifetime here; source destroys the returned
 // hardware status before clearing the recursion guard.
 virtual void send_original(std::uint32_t id,preset_bank::Value&)=0;
};
inline std::int32_t gui_cached_integer(float value)noexcept{
 // Source CVTTSS2SI, FP exception status excluded. Avoid undefined C++ casts
 // for NaN, infinity and values outside the signed32 conversion range.
 if(!std::isfinite(value)||value<-2147483648.f||value>=2147483648.f)return std::numeric_limits<std::int32_t>::min();
 return static_cast<std::int32_t>(value);
}
// Source817fb0. The actual router may move/replace incoming and return false;
// hardware forwarding still uses the original typed copy, independent of that
// result. Lock release precedes eligibility. Callbacks may change live guard.
// Stable services and serial access to owner state are required; the mutex
// protects the mode/enable query, not the complete callback or owner lifetime.
inline void gui_write_owned(GuiHardwareWriteState& state,GuiWriteLock& lock,
 GuiHardwareWriteBoundary& hardware,GuiWriteDispatchBoundary& router,
 std::uint32_t origin,std::uint32_t id,preset_bank::Value& incoming){
 if(id==state.special_id){
  state.cached_integer=gui_cached_integer(gui_parameter_scalar(&incoming));
  (void)gui_dispatch_write(origin,id,incoming,router);return;
 }
 bool set_guard=false;
 if(!state.guard&&lock.try_lock()){
  const bool eligible_mode=state.mode==1&&state.enabled;
  lock.unlock();
  if(eligible_mode&&hardware.eligible(origin,id,incoming)){state.guard=true;set_guard=true;}
 }
 auto preserved=incoming;
 (void)gui_dispatch_write(origin,id,incoming,router);
 if(state.guard&&set_guard){hardware.send_original(id,preserved);state.guard=false;}
}
}
