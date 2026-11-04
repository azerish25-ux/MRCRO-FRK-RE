#include "mfrk/fx_mix.hpp"

namespace mfrk::fx_input {
ColdConstructorDefaults cold_constructor_defaults() noexcept {
  return {GainHistory{}, PanSettings{0.0f, false, false}};
}

StereoSamples sum_lanes(const StereoLanes& lanes, StereoOverride external) noexcept {
  StereoSamples output = lanes[0];
  for (unsigned lane = 1; lane < lane_count; ++lane) {
    for (unsigned sample = 0; sample < frame_count; ++sample) {
      output.left[sample] = output.left[sample] + lanes[lane].left[sample];
      // The native first right SIMD group reverses the source operands.
      output.right[sample] = sample < 4 ? lanes[lane].right[sample] + output.right[sample]
                                        : output.right[sample] + lanes[lane].right[sample];
    }
  }
  if (external.left != nullptr && external.right != nullptr) {
    output.left = *external.left;
    output.right = *external.right;
  }
  return output;
}
}
