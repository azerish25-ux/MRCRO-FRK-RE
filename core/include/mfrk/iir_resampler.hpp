#pragma once

#include "mfrk/filter_resampling.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace mfrk {

struct IirResamplerState {
    std::array<float, 16> up;
    std::array<float, 16> down;
};

// Original filter IIR, not the oscillator's linear2x interpolator. Finite inputs,
// separate input/output ranges, whole factor-aligned blocks. No callback allocation.
class IirResampler {
public:
    explicit IirResampler(FilterOversampling factor);
    void reset() noexcept;
    void upsample(std::span<const float> input, std::span<float> output) noexcept;
    void downsample(std::span<const float> input, std::span<float> output) noexcept;
    [[nodiscard]] std::size_t factor() const noexcept;
    [[nodiscard]] std::uint32_t latency_frames() const noexcept;
    [[nodiscard]] IirResamplerState state() const noexcept;

private:
    FilterOversampling factor_;
    IirResamplerState state_{};
};

}  // namespace mfrk
