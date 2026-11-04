#include "waveshaper_test_access.hpp"
#include <cstring>
#include <iostream>
using namespace mfrk::waveshaper;
int main(){Tables tables;auto e=Engine::admitted_zero_arena(tables);std::array<float,32> output;output.fill(F(0x3eabcdef));const auto before=TestAccess::state(e);const auto sentinel=output;
 if(e.try_render_raw(output)!=RenderStatus::not_initialized||TestAccess::state(e)!=before||std::memcmp(output.data(),sentinel.data(),sizeof output))return 1;
 Input input{};input.pitch_argument=-48.0f;input.words={0,65535,0};input.dirty={1,1,1};input.output_level=F(0x3f79999a);
 if(e.try_process(input,output)!=RenderStatus::not_initialized||TestAccess::state(e)!=before||std::memcmp(output.data(),sentinel.data(),sizeof output))return 2;
 e.initialize();if(e.try_process(input,output)!=RenderStatus::ok)return 3;
 const auto saved=TestAccess::state(e);const auto saved_slope=TestAccess::slope(e),saved_fold=TestAccess::fold(e);const auto successful=output;
 input.words[2]=65535;
 for(int repeat=0;repeat<2;++repeat){if(e.try_process(input,output)!=RenderStatus::fold_domain)return 4;if(TestAccess::state(e)!=saved||std::memcmp(output.data(),successful.data(),sizeof output)||std::memcmp(TestAccess::slope(e).data(),saved_slope.data(),sizeof saved_slope)||std::memcmp(TestAccess::fold(e).data(),saved_fold.data(),sizeof saved_fold))return 5;}
 TestAccess::prepare(e,input);const auto prepared=TestAccess::state(e);if(e.try_render_raw(output)!=RenderStatus::fold_domain||TestAccess::state(e)!=prepared||std::memcmp(output.data(),successful.data(),sizeof output))return 6;
 const auto checked_state=TestAccess::state(e);if(e.try_set_control_word(3,1)||e.try_set_control_dirty(999,1)||TestAccess::state(e)!=checked_state)return 7;
 auto safe=Engine::admitted_zero_arena(tables);safe.initialize();input.pitch_argument=0;
 for(int i=0;i<512;++i){input.words[2]=i%2?65535:0;if(safe.try_process(input,output)!=RenderStatus::ok)return 8;}
 std::cout<<"{\"not_initialized_checks\":2,\"transactional_fold_failures\":3,\"invalid_setter_checks\":2,\"defined_stress_blocks\":512,\"mismatches\":0}\n";return 0;
}
