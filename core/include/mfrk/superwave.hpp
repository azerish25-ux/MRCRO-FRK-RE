#pragma once

#include "mfrk/basic_wave_tables.hpp"

#include <array>
#include <cstdint>
#include <span>

namespace mfrk {

struct SuperWaveTableSelection {
    std::uint32_t family = 3, band = 0;
};

struct SuperWaveState {
    // Must be supplied: native constructor leaves input words and flags untouched.
    std::array<std::uint16_t, 3> words;
    std::array<std::uint8_t, 3> dirty;
    std::array<SuperWaveTableSelection, 7> tables{};
    std::array<std::uint32_t, 7> bands{}, families{}, phase{}, increment{}, seed{};
    // Storage only: render requires the first full prepare to establish weights.
    std::array<std::int16_t, 7> weight{};
    bool coefficients_initialized = false;
    std::uint16_t wave_quartile = 0;
    std::int16_t blend_fixed = 0;
    float detune = 0, frequency = 0, table_key = 0, blend = 0;
    float target = 0, current, coefficient, threshold;
    std::uint8_t force = 0, trigger_suppressed = 1;
    float level;

    static SuperWaveState admitted(std::array<std::uint16_t, 3> words,
                                   std::array<std::uint8_t, 3> dirty) noexcept;
};

void render_superwave32(SuperWaveState& state,
                       const mfrk::BasicWaveTables::Table& table,
                       std::span<float, 32> output) noexcept;

// Finite inputs and representable float-to-int64 conversions are required.
// Disable fast-math/FP contraction. Shared tables must outlive this object.
class SuperWave {
    const mfrk::BasicWaveTables& tables_;
    SuperWaveState state_;

public:
    SuperWave(const mfrk::BasicWaveTables& tables, SuperWaveState admitted) noexcept
        : tables_(tables), state_(admitted) {}

    const SuperWaveState& state() const noexcept { return state_; }

    void set_inputs(std::array<std::uint16_t, 3> words,
                    std::array<std::uint8_t, 3> dirty) noexcept {
        state_.words = words;
        state_.dirty = dirty;
    }

    // Native aggregate2e0fa0 stores the literal byte at raw+0x141.
    void set_trigger_suppressed(std::uint8_t value) noexcept { state_.trigger_suppressed = value; }

    void set_pitch(float hz, float table_key) noexcept;
    // Phase plus each offset must have a representable signed32 truncation.
    // Native0x1802c63d0 updates seven seeds AND seven current phases.
    void set_phase(float phase) noexcept;
    void set_phase_seeds(std::array<std::uint32_t, 7> seeds) noexcept {
        state_.seed = seeds;
    }
    // Native phase restore and LPF setBoth; no coefficient recomputation.
    void initialize() noexcept;
    void repoint() noexcept;
    void trigger(std::uint8_t flag) noexcept;
    void prepare_controls(bool clear_dirty = false) noexcept;
    void render_raw(std::span<float, 32> output) noexcept;
    void render_scaled(std::span<float, 32> output) noexcept;
};

}  // namespace mfrk
