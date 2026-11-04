#include "mfrk/waveshaper.hpp"
#include <type_traits>
#include <utility>
using namespace mfrk::waveshaper;
static_assert(!std::is_default_constructible_v<Engine>);
static_assert(noexcept(Engine::admitted_zero_arena(std::declval<const Tables&>())));
int main(){Tables tables;auto engine=Engine::admitted_zero_arena(tables);std::array<float,32> output{};return engine.try_render_raw(output)==RenderStatus::not_initialized?0:1;}
