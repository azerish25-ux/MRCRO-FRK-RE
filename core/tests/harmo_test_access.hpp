#pragma once
#include "mfrk/harmo.hpp"
#include <algorithm>
namespace mfrk::harmo {
struct TestAccess {
    static const auto& state(const Engine& engine) {return engine.state;}
    static auto& state(Engine& engine) {return engine.state;}
    static auto delay(const Engine& engine) {std::array<std::int16_t,2048> result;const auto view=engine.delay_words();std::copy(view.begin(),view.end(),result.begin());return result;}
    static const auto& norm(const Tables& tables) {return tables.norm;}
    static const auto& auxiliary(const Tables& tables) {return tables.auxiliary;}
};
}
