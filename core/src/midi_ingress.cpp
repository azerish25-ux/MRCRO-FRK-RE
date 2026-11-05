#include "mfrk/midi_ingress.hpp"

namespace mfrk::midi {
static_assert(std::atomic<std::uint64_t>::is_always_lock_free);
bool Ingress::push(const Message& message) noexcept {
 if(overflow_.load(std::memory_order_acquire))return false;
 const auto write=write_.load(std::memory_order_relaxed);
 const auto read=read_.load(std::memory_order_acquire);
 if(write-read>=capacity){overflow_.store(true,std::memory_order_release);return false;}
 queue_[write%capacity]=message;
 write_.store(write+1,std::memory_order_release);
 return true;
}
void Ingress::emit(void* opaque,const Event& event) noexcept {
 auto& context=*static_cast<CallbackContext*>(opaque);
 if(!context.owner->push(Message{context.device,context.port,context.timestamp,event,false}))context.ok=false;
}
bool Ingress::feed(std::int32_t device,std::int32_t port,std::span<const std::uint8_t> bytes,std::int64_t timestamp) noexcept {
 Source* found=nullptr;Source* free=nullptr;
 for(auto& source:sources_){if(source.active&&source.device==device&&source.port==port){found=&source;break;}if(!source.active&&!free)free=&source;}
 if(!found){if(!free){overflow_.store(true,std::memory_order_release);return false;}found=free;found->device=device;found->port=port;found->active=true;found->decoder.reset();}
 CallbackContext context{this,device,port,timestamp,true};
 found->decoder.feed(bytes,emit,&context);
 return context.ok;
}
bool Ingress::reset_port(std::int32_t device,std::int32_t port) noexcept {
 for(auto& source:sources_)if(source.active&&source.device==device&&source.port==port){source.decoder.reset();source.active=false;break;}
 return push(Message{device,port,0,{},true});
}
bool Ingress::pop(Message& result) noexcept {
 if(overflow_.load(std::memory_order_acquire))return false;
 const auto read=read_.load(std::memory_order_relaxed);
 if(read==write_.load(std::memory_order_acquire))return false;
 result=queue_[read%capacity];read_.store(read+1,std::memory_order_release);return true;
}
bool Ingress::recover_overflow() noexcept {
 if(!overflow_.load(std::memory_order_acquire))return false;
 read_.store(write_.load(std::memory_order_acquire),std::memory_order_release);
 overflow_.store(false,std::memory_order_release);
 return true;
}
}
