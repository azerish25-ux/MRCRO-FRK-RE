#include "mfrk/fx_input.hpp"
#include "fx_input_constants.hpp"
#include "ucrt_sinf.hpp"
#include <algorithm>
#include <bit>
#include <cmath>

namespace mfrk::fx_input {
namespace {
std::int16_t wrap16(std::int32_t value) noexcept {
  return std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(value));
}
}

StereoLanes InputGainPan::process(const MonoInputs& mono, const Targets& envelopes,
                                 const Targets& pans, PanSettings settings) noexcept {
  StereoLanes output;
  const auto& pattern = settings.alternate_pattern ? detail::pattern1 : detail::pattern0;
  const float spread = settings.disable_spread ? 0.0f : settings.width;
  for (unsigned lane = 0; lane < lane_count; ++lane) {
    const float offset = spread * pattern[lane];
    float pan = offset + pans[lane];
    if (pan > 1.0f) {
      pan = 2.0f - pan;
    } else if (pan < -1.0f) {
      pan = -2.0f - pan;
    }
    const float amplitude = envelopes[lane] * detail::calibration;
    const float angle = pan * detail::quarter;
    const float target_right = mfrk::detail::ucrt_sinf::evaluate(angle + detail::quarter) * amplitude;
    const float target_left = mfrk::detail::ucrt_sinf::evaluate(angle + detail::three_quarter) * amplitude;
    const float difference_left = target_left - history_[lane].left;
    const float difference_right = target_right - history_[lane].right;
    for (unsigned sample = 0; sample < frame_count; ++sample) {
      const float fraction = float(sample) * 0.0625f;
      const float left_step = difference_left * fraction;
      const float right_step = difference_right * fraction;
      const float left_gain = left_step + history_[lane].left;
      const float right_gain = right_step + history_[lane].right;
      output[lane].left[sample] = left_gain * mono[lane][sample];
      output[lane].right[sample] = right_gain * mono[lane][sample];
    }
    history_[lane] = {target_left, target_right};
  }
  return output;
}

std::int16_t envelope_source_q15(std::int16_t adsr, std::uint8_t velocity,
                                std::int16_t sensitivity) noexcept {
  const auto velocity_term = wrap16((std::int32_t(sensitivity) * velocity) >> 7);
  const auto factor = wrap16(std::int32_t(velocity_term) - sensitivity + 16383);
  return wrap16((std::int32_t(factor) * adsr) >> 14);
}

std::int16_t vca_gain_q14(std::int16_t source, std::uint8_t velocity,
                         std::int16_t sensitivity, std::int16_t effective) noexcept {
  constexpr std::int32_t unit = 16383;
  const auto factor = ((std::int32_t(sensitivity) * velocity) >> 7) - sensitivity + unit;
  const auto envelope_and_velocity = wrap16((std::int32_t(source) * factor) >> 15);
  const auto result = (std::int32_t(envelope_and_velocity) * effective) >> 14;
  return static_cast<std::int16_t>(std::clamp(result, 0, 16383));
}

std::uint16_t canonical_word_from_q14(std::int16_t input) noexcept {
  const auto clamped = std::clamp<std::int32_t>(input, 0, 16383);
  return static_cast<std::uint16_t>(clamped * 4);
}

float normalized_gain_from_q14(std::int16_t input) noexcept {
  constexpr float scale = std::bit_cast<float>(std::uint32_t{0x37800200});
  return float(canonical_word_from_q14(input)) * scale;
}
}
