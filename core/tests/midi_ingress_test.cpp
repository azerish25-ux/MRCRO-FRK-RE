#include "mfrk/midi_ingress.hpp"
#include <array>
#include <atomic>
#include <thread>
using namespace mfrk::midi;
int main(){
 Ingress ingress;Message message{};
 const std::array<std::uint8_t,2> a{0x90,60};const std::array<std::uint8_t,2> b{0x91,70};
 const std::array<std::uint8_t,1> va{100},vb{110};
 if(!ingress.feed(1,0,a,10)||!ingress.feed(2,0,b,20)||!ingress.feed(1,0,va,30)||!ingress.feed(2,0,vb,40))return 1;
 if(!ingress.pop(message)||message.device!=1||message.event.status!=0x90||message.event.data!=std::array<std::uint8_t,2>{60,100}||message.timestamp!=30)return 2;
 if(!ingress.pop(message)||message.device!=2||message.event.status!=0x91||message.event.data!=std::array<std::uint8_t,2>{70,110}||message.timestamp!=40)return 3;
 if(!ingress.reset_port(1,0)||!ingress.pop(message)||!message.port_reset||message.device!=1)return 4;
 const std::array<std::uint8_t,1> realtime{0xf8};
 for(unsigned i=0;i<Ingress::capacity+1;++i)ingress.feed(2,0,realtime,50);
 if(!ingress.recover_overflow()||ingress.pop(message)||ingress.recover_overflow())return 5;
 if(!ingress.feed(2,0,realtime,60)||!ingress.pop(message)||message.timestamp!=60)return 6;
 Ingress concurrent;std::atomic<bool> done=false;std::atomic<bool> failed=false;
 std::thread producer([&]{for(int i=0;i<3000;++i)if(!concurrent.feed(7,1,realtime,i))failed=true;done=true;});
 int next=0;while(!done.load()||next<3000){if(concurrent.pop(message)){if(message.device!=7||message.port!=1||message.timestamp!=next||message.event.status!=0xf8)failed=true;++next;}else if(done.load()&&next<3000){failed=true;break;}}
 producer.join();if(failed||next!=3000||concurrent.recover_overflow())return 7;
}
