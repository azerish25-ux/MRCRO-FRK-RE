#pragma once
#include "mfrk/adsr.hpp"
namespace mfrk {
struct AdsrTestAccess {
    static void load_explicit_fixture(Adsr& env, const AdsrState& state) noexcept {
      env.state_=state;
      env.prepared_mask_=0x1ff;
      env.controls_={};
    }
  };
}
