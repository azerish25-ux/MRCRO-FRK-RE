#include "mfrk/fir_resampler.hpp"
#include "fir_coefficients.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <stdexcept>

namespace mfrk {
namespace {
std::span<const float> taps_for(FilterOversampling factor) {
    switch (factor) {
    case FilterOversampling::bypass: return {};
    case FilterOversampling::x2: return detail::kFilterFir2;
    case FilterOversampling::fallback_x3: return detail::kFilterFir3;
    case FilterOversampling::x4: return detail::kFilterFir4;
    case FilterOversampling::x6: return detail::kFilterFir6;
    case FilterOversampling::x8: return detail::kFilterFir8;
    }
    throw std::invalid_argument("Unsupported original FIR factor");
}

std::size_t checked_workspace(std::size_t history, std::size_t frames, std::size_t factor) {
    if (frames == 0 || frames > (std::numeric_limits<std::size_t>::max() - history) / factor) {
        throw std::length_error("Invalid original FIR capacity");
    }
    return history + frames * factor;
}
}  // namespace

FirResampler::FirResampler(FilterOversampling factor, std::size_t maximum_frames)
    : factor_(static_cast<std::size_t>(factor)), maximum_frames_(maximum_frames) {
    const auto taps = taps_for(factor);
    const auto history = ((std::max(taps.size(), std::size_t{4}) + factor_ - 1) / factor_) * factor_;
    phase_length_ = ((history / factor_ + 3) / 4) * 4;
    phase_coefficients_.resize(factor_ * phase_length_);
    for (std::size_t i = 0; i < taps.size(); ++i) {
        const auto q = i / factor_;
        phase_coefficients_[(i % factor_) * phase_length_ + (q & ~std::size_t{3}) + (3 - (q & 3))] = taps[i];
    }
    const auto padded = std::max(std::size_t{4}, ((taps.size() + 3) / 4) * 4);
    down_coefficients_.resize(padded);
    for (std::size_t i = 0; i < taps.size(); ++i) down_coefficients_[padded - 1 - i] = taps[i];
    up_history_.resize(history);
    down_history_.resize(padded - 1);
    up_workspace_.resize(checked_workspace(history, maximum_frames_, 1));
    down_workspace_.resize(checked_workspace(padded - 1, maximum_frames_, factor_));
}

void FirResampler::reset() noexcept {
    std::fill(up_history_.begin(), up_history_.end(), 0.0f);
    std::fill(down_history_.begin(), down_history_.end(), 0.0f);
}

void FirResampler::upsample(std::span<const float> input, std::span<float> output) noexcept {
    assert(input.size() <= maximum_frames_ && output.size() / factor_ == input.size() && output.size() % factor_ == 0);
    if (factor_ == 1) { std::copy(input.begin(), input.end(), output.begin()); return; }
    std::copy(up_history_.begin(), up_history_.end(), up_workspace_.begin());
    std::copy(input.begin(), input.end(), up_workspace_.begin() + up_history_.size());
    for (std::size_t n = 0; n < input.size(); ++n) {
        const auto current = up_history_.size() + n;
        for (std::size_t phase = 0; phase < factor_; ++phase) {
            const auto* coefficients = phase_coefficients_.data() + phase * phase_length_;
            std::array<float, 4> sums;
            for (std::size_t lane = 0; lane < 4; ++lane) sums[lane] = up_workspace_[current - 3 + lane] * coefficients[lane];
            for (std::size_t group = 1; group < phase_length_ / 4; ++group) {
                for (std::size_t lane = 0; lane < 4; ++lane) {
                    sums[lane] = sums[lane] + up_workspace_[current - (group * 4 + 3 - lane)] * coefficients[group * 4 + lane];
                }
            }
            output[n * factor_ + phase] = (((sums[0] + sums[1]) + sums[2]) + sums[3]) * static_cast<float>(factor_);
        }
    }
    std::copy_n(up_workspace_.begin() + input.size(), up_history_.size(), up_history_.begin());
}

void FirResampler::downsample(std::span<const float> input, std::span<float> output) noexcept {
    assert(output.size() <= maximum_frames_ && input.size() / factor_ == output.size() && input.size() % factor_ == 0);
    if (factor_ == 1) { std::copy(input.begin(), input.end(), output.begin()); return; }
    std::copy(down_history_.begin(), down_history_.end(), down_workspace_.begin());
    std::copy(input.begin(), input.end(), down_workspace_.begin() + down_history_.size());
    for (std::size_t n = 0; n < output.size(); ++n) {
        std::array<float, 4> sums{};
        for (std::size_t group = 0; group < down_coefficients_.size() / 4; ++group) {
            for (std::size_t lane = 0; lane < 4; ++lane) {
                const auto i = group * 4 + lane;
                sums[lane] = sums[lane] + down_workspace_[n * factor_ + i] * down_coefficients_[i];
            }
        }
        output[n] = ((sums[0] + sums[1]) + sums[2]) + sums[3];
    }
    std::copy_n(down_workspace_.begin() + input.size(), down_history_.size(), down_history_.begin());
}

std::size_t FirResampler::factor() const noexcept { return factor_; }
std::uint32_t FirResampler::latency_frames() const noexcept { return factor_ == 1 || factor_ == 3 ? 0 : 48; }
}  // namespace mfrk
