#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
namespace mfrk::ui {
struct GuiAsyncUpdateTarget {
 virtual ~GuiAsyncUpdateTarget()=default;
 virtual void handle_async_update()=0;
};
// Source AsyncUpdaterMessage f7c7c0. Queue entries retain this message and its
// pending word, not the updater/target. The target's owner cancels before
// destruction; pending0 prevents any subsequent target dereference.
class GuiAsyncUpdateMessage final {
 GuiAsyncUpdateTarget& target_;
public:
 std::atomic<std::uint32_t> pending{0};
 explicit GuiAsyncUpdateMessage(GuiAsyncUpdateTarget& target):target_(target){}
 void deliver(){std::uint32_t expected=1;if(pending.compare_exchange_strong(expected,0))target_.handle_async_update();}
};
struct GuiAsyncUpdateQueue {
 virtual ~GuiAsyncUpdateQueue()=default;
 // The actual queue retains a message for every successful post. It delivers
 // on the UI thread, without synchronous callbacks from this posting method.
 virtual bool post(std::shared_ptr<GuiAsyncUpdateMessage>)=0;
};
// Source trigger/cancel/post ownership: one message object per updater. Repeated
// queue entries may reference it after cancellation/retriggering. Destruction
// cancels then drops the owner's reference; queued entries retain the word.
class GuiAsyncUpdater final {
 GuiAsyncUpdateQueue& queue_;
 std::shared_ptr<GuiAsyncUpdateMessage> message_;
public:
 GuiAsyncUpdater(GuiAsyncUpdateTarget& target,GuiAsyncUpdateQueue& queue)
  :queue_(queue),message_(std::make_shared<GuiAsyncUpdateMessage>(target)){}
 GuiAsyncUpdater(const GuiAsyncUpdater&)=delete;
 GuiAsyncUpdater& operator=(const GuiAsyncUpdater&)=delete;
 ~GuiAsyncUpdater(){cancel();}
 std::atomic<std::uint32_t>& pending_word()noexcept{return message_->pending;}
 bool is_pending()const noexcept{return message_->pending.load()!=0;}
 bool post_message(){return queue_.post(message_);}
 void trigger(){std::uint32_t expected=0;if(message_->pending.compare_exchange_strong(expected,1))if(!post_message())message_->pending.exchange(0);}
 void cancel()noexcept{message_->pending.exchange(0);}
 // Message-body delivery and target destruction are serialized on the UI
 // thread. Producers must stop using the updater before its destruction.
};
struct GuiAsyncMouseState {std::uint8_t buttons=0;};
// Source ff8b50 with the actual main-mouse state supplied by the input owner.
inline void gui_component_fake_mouse_move(std::uint8_t component_flags,const GuiAsyncMouseState& mouse,GuiAsyncUpdater& updater){
 if((component_flags&8u)!=0&&(component_flags&0x10u)==0)return;
 if((mouse.buttons&0x70u)==0)updater.trigger();
}
}
