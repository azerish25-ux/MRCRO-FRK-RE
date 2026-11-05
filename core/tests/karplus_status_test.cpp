#include "karplus_test_access.hpp"
#include <iostream>
using namespace mfrk::karplus;
int main(){Tables t;SharedRandom r{1};Memory m{};auto e=Engine::admitted_zero_arena(t,r,m);std::array<float,32>out;out.fill(F(0x7fa12345));unsigned tests=0;
const auto check=[&](Status expected){const auto state=TestAccess::state(e);const auto buffers=m.samples;const auto seed=r.value;const auto raw=TestAccess::raw(e),excite=TestAccess::excite(e);const auto output=std::bit_cast<std::array<std::uint32_t,32>>(out);const auto status=e.try_render(out);++tests;return status==expected&&state==TestAccess::state(e)&&buffers==m.samples&&seed==r.value&&raw==TestAccess::raw(e)&&excite==TestAccess::excite(e)&&output==std::bit_cast<std::array<std::uint32_t,32>>(out);};
if(!check(Status::not_initialized)){return 1;}e.initialize();if(!check(Status::unbound))return 1;e.bind(0x400020000,8192);e.initialize();e.set_pitch(440,69);e.trigger(true);for(int i=0;i<17;++i)if(e.try_render(out)!=Status::ok)return 1;
TestAccess::write(e,0x88,-200.0f);if(!check(Status::table_domain))return 1;
e.set_pitch(440,69);TestAccess::write(e,0x90,std::int32_t{0});TestAccess::write(e,0x94,std::int32_t{16});TestAccess::write(e,0x98,1e20f);TestAccess::write(e,0x9c,1e20f);if(!check(Status::math_domain))return 1;
const auto before=TestAccess::state(e);if(e.set_word(3,123)||e.set_dirty(3,123)||before!=TestAccess::state(e))return 1;++tests;
Input in{};in.frequency_hz=440;in.note=69;in.trigger_event=3;const auto output=std::bit_cast<std::array<std::uint32_t,32>>(out);if(e.try_process(in,out)!=Status::invalid_input||before!=TestAccess::state(e)||output!=std::bit_cast<std::array<std::uint32_t,32>>(out))return 1;++tests;
// A failure late in processing rolls back setter/preparation writes as well.
in.trigger_event=0;in.words={65535,0,65535};in.dirty={1,1,1};const auto seed=r.value;const auto buffers=m.samples;if(e.try_process(in,out)!=Status::math_domain||before!=TestAccess::state(e)||seed!=r.value||buffers!=m.samples||output!=std::bit_cast<std::array<std::uint32_t,32>>(out))return 1;++tests;
std::cout<<"release rollback/admission checks "<<tests<<" passed\n";
}

