#include "mfrk/midi_stream.hpp"
#include <array>
#include <vector>
using namespace mfrk::midi;
static void collect(void* context,const Event& event) noexcept {static_cast<std::vector<Event>*>(context)->push_back(event);}
int main(){
 Decoder decoder;std::vector<Event> events;events.reserve(16);
 const std::array<std::uint8_t,4> first{0x90,60,0xf8,100};
 const std::array<std::uint8_t,8> second{61,0,0xc2,7,8,0xf2,1,2};
 decoder.feed(first,collect,&events);decoder.feed(second,collect,&events);
 const std::vector<Event> expected{{0xf8,{},0,false},{0x90,{60,100},2,false},{0x90,{61,0},2,false},{0xc2,{7,0},1,false},{0xc2,{8,0},1,false},{0xf2,{1,2},2,false}};
 if(events!=expected)return 1;
 events.clear();decoder.reset();
 const std::array<std::uint8_t,7> sysex{0xf0,0x7d,0xf8,1,0xf7,0x90,60};
 decoder.feed(sysex,collect,&events);const std::array<std::uint8_t,1> last{127};decoder.feed(last,collect,&events);
 const std::vector<Event> expected_sysex{{0xf0,{},0,true},{0x7d,{},0,true},{0xf8,{},0,false},{1,{},0,true},{0xf7,{},0,true},{0x90,{60,127},2,false}};
 if(events!=expected_sysex)return 2;
 events.clear();decoder.reset();const std::array<std::uint8_t,8> abort{0x90,60,0xf1,3,61,100,0xf6,0xff};decoder.feed(abort,collect,&events);
 const std::vector<Event> expected_abort{{0xf1,{3,0},1,false},{0xf6,{},0,false},{0xff,{},0,false}};
 return events==expected_abort?0:3;
}
