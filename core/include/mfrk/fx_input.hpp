#pragma once
#include <array>
#include <cstdint>
namespace mfrk::fx_input {
inline constexpr unsigned lane_count = 7;
inline constexpr unsigned frame_count = 16;
using Samples = std::array<float, frame_count>;
using MonoInputs = std::array<Samples, lane_count>;
using Targets = std::array<float, lane_count>;
struct GainPair { float left; float right; };
using GainHistory = std::array<GainPair, lane_count>;
struct StereoSamples { Samples left; Samples right; };
using StereoLanes = std::array<StereoSamples, lane_count>;
struct PanSettings {
  float width;
  bool disable_spread;
  bool alternate_pattern;
};
// Finite mono samples/envelopes/pan/settings; round-to-nearest floating point.
// process() requires each computed sine argument to satisfy abs(float bits)
// < 0x4b800456 (the recovered UCRT ordinary-domain branch). Canonical
// pans in [-1,1] and width in [0,1] satisfy this requirement.
// Canonical controller helpers:
// signed16 source, Q14 sensitivities/effective0..16383, velocity0..127.
class InputGainPan {
public:
  // Starting gains must come from recovered caller state or an explicit fixture.
  // The original FX constructor's gain history/default flags are not assumed.
  explicit InputGainPan(GainHistory history) noexcept : history_(history) {}
  [[nodiscard]] StereoLanes process(const MonoInputs&, const Targets& envelopes,
                                   const Targets& pans, PanSettings) noexcept;
  [[nodiscard]] const GainHistory& history() const noexcept { return history_; }
private:
  GainHistory history_;
};
[[nodiscard]] std::int16_t envelope_source_q15(std::int16_t adsr_q15,
    std::uint8_t velocity, std::int16_t velocity_envelope_q14) noexcept;
[[nodiscard]] std::int16_t vca_gain_q14(std::int16_t envelope_source,
    std::uint8_t velocity, std::int16_t velocity_vca_q14,
    std::int16_t post_matrix_effective_q14) noexcept;
[[nodiscard]] std::uint16_t canonical_word_from_q14(std::int16_t gain_q14) noexcept;
[[nodiscard]] float normalized_gain_from_q14(std::int16_t gain_q14) noexcept;
}
