#pragma once

#include "mfrk/basic_wave_tables.hpp"

#include <memory>
#include <span>

namespace mfrk {

// Recovered raw Basic Waves operations. The table object must outlive this
// instance. Pitch and phase inputs must have representable int64 conversions.
// Host scheduling, voice initialization and output level are separate.
class BasicWaves {
public:
    explicit BasicWaves(const BasicWaveTables& tables);
    ~BasicWaves();
    BasicWaves(const BasicWaves&) = delete;
    BasicWaves& operator=(const BasicWaves&) = delete;

    void set_pitch(float frequency_hz, float table_key) noexcept;
    void set_phase_seed(float phase) noexcept;
    void reset() noexcept;
    void prepare_controls(const std::array<std::uint16_t, 3>& words,
                          const std::array<std::uint8_t, 3>& dirty) noexcept;
    void render(std::span<float, 32> output) noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace mfrk
