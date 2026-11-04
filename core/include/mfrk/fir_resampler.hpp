#pragma once

#include "mfrk/filter_resampling.hpp"

#include <cstddef>
#include <vector>

namespace mfrk {

// Original FIR source taps, phase packing, four-lane reduction and histories.
// Capacity allocated at construction. Finite separate input/output ranges;
// upsample output=F*input, downsample input=F*output, input host count<=capacity.
class FirResampler {
public:
    FirResampler(FilterOversampling factor, std::size_t maximum_frames);
    void reset() noexcept;
    void upsample(std::span<const float> input, std::span<float> output) noexcept;
    void downsample(std::span<const float> input, std::span<float> output) noexcept;
    [[nodiscard]] std::size_t factor() const noexcept;
    [[nodiscard]] std::uint32_t latency_frames() const noexcept;
private:
    std::size_t factor_, maximum_frames_, phase_length_;
    std::vector<float> phase_coefficients_, down_coefficients_;
    std::vector<float> up_history_, down_history_, up_workspace_, down_workspace_;
};

}  // namespace mfrk
