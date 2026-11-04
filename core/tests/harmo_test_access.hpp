#pragma once
#include "mfrk/harmo.hpp"
namespace mfrk::harmo {
struct TestAccess {
    static const auto& state(const Engine& engine) {return engine.state;}
    static auto& state(Engine& engine) {return engine.state;}
    static const auto& delay(const Engine& engine) {return engine.delay;}
    static const auto& norm(const Tables& tables) {return tables.norm;}
    static const auto& auxiliary(const Tables& tables) {return tables.auxiliary;}
};
}
