#include "mfrk/filter_resampling.hpp"

#include <algorithm>
#include <cassert>

namespace mfrk {
FilterOversampling select_filter_factor(std::uint32_t index, bool adaptive, float rate) noexcept {
    constexpr std::array<std::uint32_t, 4> factors{2, 4, 6, 8};
    assert(index < factors.size());
    auto factor = factors[index];
    if (adaptive) {
        const auto shift = rate >= 352800.0f ? 3 : rate >= 176400.0f ? 2 : rate >= 88200.0f ? 1 : 0;
        factor = std::max(std::uint32_t{1}, factor >> shift);
    }
    return static_cast<FilterOversampling>(factor);
}

FilterFade::FilterFade(float rate, float time, float scale, std::uint32_t count, float initial) noexcept
    : current_(initial), target_(initial), step_((scale * static_cast<float>(count)) / (rate * time)) {}

void FilterFade::set_target(float target) noexcept { target_ = target; }
void FilterFade::set_both(float value) noexcept { current_ = target_ = value; }

void FilterFade::render(std::span<float, 32> output) noexcept {
    if (current_ == target_) { std::fill(output.begin(), output.end(), target_); return; }
    const float difference = target_ - current_;
    const float stride = step_ * 4.0f;
    std::array<float, 4> offsets{step_, step_ * 2.0f, step_ * 3.0f, step_ * 4.0f};
    const bool rising = difference > 0.0f;
    const bool clamp = rising ? difference < 32.0f * step_ : difference > -(32.0f * step_);
    for (std::size_t group = 0; group < 8; ++group) {
        for (std::size_t lane = 0; lane < 4; ++lane) {
            const float raw = rising ? current_ + offsets[lane] : current_ - offsets[lane];
            const bool before_target = rising ? offsets[lane] < difference : difference < -offsets[lane];
            output[group * 4 + lane] = clamp && !before_target ? target_ : raw;
        }
        for (auto& offset : offsets) offset = offset + stride;
    }
    current_ = output.back();
}

FilterFadeState FilterFade::state() const noexcept { return {current_, target_, step_}; }
}  // namespace mfrk
