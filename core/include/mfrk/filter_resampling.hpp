#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace mfrk {

enum class FilterOversampling : std::uint32_t {
    bypass = 1, x2 = 2, fallback_x3 = 3, x4 = 4, x6 = 6, x8 = 8
};
enum class FilterKernel : std::uint32_t { iir, fir };

// Index0..3 selects original2/4/6/8 before the original rate-dependent shift.
// This internal selection is distinct from GUI lowpass/bandpass/highpass mode.
[[nodiscard]] FilterOversampling select_filter_factor(std::uint32_t index,
    bool adaptive, float base_rate) noexcept;

struct FilterFadeState { float current, target, step; };

// Original four-lane vector fade for32-sample blocks; positive rate/time and
// finite controls. Construction/reconfiguration belong outside the callback.
class FilterFade {
public:
    FilterFade(float base_rate, float time, float scale,
               std::uint32_t block_count, float initial) noexcept;
    void set_target(float target) noexcept;
    void set_both(float value) noexcept;
    void render(std::span<float, 32> output) noexcept;
    [[nodiscard]] FilterFadeState state() const noexcept;
private:
    float current_, target_, step_;
};

}  // namespace mfrk
