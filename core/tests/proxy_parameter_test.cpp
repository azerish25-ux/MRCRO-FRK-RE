#include "mfrk/proxy_parameter.hpp"
#include "mfrk/preset_bank.hpp"
#include <array>
using namespace mfrk;
struct Boundary {
 std::uint32_t value=0;std::vector<std::uint32_t> reads;std::vector<std::array<std::uint32_t,3>> writes;
 std::uint32_t get(std::uint32_t id){reads.push_back(id);return value;}
 void write(std::uint32_t origin,std::uint32_t id,std::uint32_t bits){writes.push_back({origin,id,bits});}
};
int main(){
 preset_bank::BankPair banks;for(auto* bank:{&banks.active(),&banks.prepared()})bank->internal.push_back({0x80000000u,preset_bank::Value::scalar(0),false});
 const std::array<preset_program::Write,1> writes{{{0x80000000u,1,{0,0,0,63},true,{}}}};if(!preset_bank::stage(banks.prepared(),writes))return 1;banks.activate_main();
 const std::array<std::uint32_t,1> flags{2};auto commands=preset_bank::notifications(banks.active(),flags,{});if(!commands||commands->size()!=1)return 2;
 const std::array<std::uint32_t,3> aliases{0x80000000u,0x80000002u,0x80000001u};Boundary b;
 if(!proxy_parameter::notify((*commands)[0],aliases,19,b)||b.reads!=std::vector<std::uint32_t>{0x80000002u,0x80000001u}||b.writes!=std::vector<std::array<std::uint32_t,3>>{{19,0x80000002u,0x3f000000u},{19,0x80000001u,0x3f000000u}})return 3;
 b={};auto command=(*commands)[0];command.origin=19;if(!proxy_parameter::notify(command,aliases,19,b)||!b.reads.empty()||!b.writes.empty())return 4;
 b={};b.value=0x7f812345u;if(!proxy_parameter::notify((*commands)[0],aliases,19,b)||b.reads.size()!=2||!b.writes.empty())return 5;
 b={};command.origin=4;command.payload={0,0,0,128};if(!proxy_parameter::notify(command,aliases,19,b)||!b.writes.empty())return 6;
 b={};command.tag=3;if(proxy_parameter::notify(command,aliases,19,b)||!b.reads.empty())return 7;
 command.tag=1;command.payload.pop_back();if(proxy_parameter::notify(command,aliases,19,b)||!b.reads.empty())return 8;
}
