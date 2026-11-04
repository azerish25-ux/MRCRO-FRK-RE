#pragma once
#include "mfrk/fx_input.hpp"

namespace mfrk::fx_input {
struct StereoOverride {
  const Samples* left;
  const Samples* right;
};

// Mirrors fields immediately after the original kernel constructor stores.
// Does not represent post-prepare settings or a history-reset operation.
struct ColdConstructorDefaults {
  GainHistory history;
  PanSettings pan;
};

[[nodiscard]] ColdConstructorDefaults cold_constructor_defaults() noexcept;

// Normal FX304250 passes {nullptr, nullptr}. A single non-null override is ignored.
// Input7/effect-side audio and the subsequent master/effects stage are separate.
[[nodiscard]] StereoSamples sum_lanes(const StereoLanes&, StereoOverride) noexcept;
}
