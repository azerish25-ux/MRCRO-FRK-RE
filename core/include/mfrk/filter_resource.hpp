#pragma once

#include "mfrk/filter_resampling.hpp"
#include "mfrk/vcf49.hpp"

#include <cstddef>
#include <memory>

namespace mfrk {

// Numeric resource only: independent audio/modulation resampling and buffers.
// The raw filter is owned by its wrapper, including across deferred replacement.
class FilterResource {
public:
    FilterResource(FilterOversampling factor, FilterKernel kernel, std::size_t maximum_frames);
    ~FilterResource();
    FilterResource(const FilterResource&) = delete;
    FilterResource& operator=(const FilterResource&) = delete;
    void reset() noexcept;
    void process(Vcf49& filter, std::span<const float> audio,
                 std::span<const float> modulation, std::span<float> output) noexcept;
    [[nodiscard]] std::size_t factor() const noexcept;
    [[nodiscard]] std::uint32_t latency_frames() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace mfrk
