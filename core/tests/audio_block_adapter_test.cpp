#include "mfrk/audio_block_adapter.hpp"
#include <array>
#include <cstddef>
using namespace mfrk::audio;
struct Engine {unsigned next=0,blocks=0;bool fail=false;};
static bool render(void* opaque,std::span<float,16> left,std::span<float,16> right) noexcept {
 auto& engine=*static_cast<Engine*>(opaque);if(engine.fail)return false;++engine.blocks;
 for(unsigned i=0;i<16;++i){left[i]=float(engine.next);right[i]=-float(engine.next);++engine.next;}return true;
}
int main(){
 Engine engine;BlockAdapter adapter(render,&engine);std::array<float,74> output{};
 if(!adapter.fill(std::span(output).first<6>())||!adapter.fill(std::span(output).subspan<6,34>())||!adapter.fill(std::span(output).subspan<40,34>()))return 1;
 if(engine.blocks!=3)return 2;for(unsigned i=0;i<37;++i)if(output[2*i]!=float(i)||output[2*i+1]!=-float(i))return 3;
 engine.fail=true;std::array<float,32> tail{};if(adapter.fill(tail))return 4;
 for(unsigned i=0;i<11;++i)if(tail[2*i]!=float(i+37)||tail[2*i+1]!=-float(i+37))return 5;
 for(unsigned i=22;i<32;++i)if(tail[i]!=0)return 6;
}
