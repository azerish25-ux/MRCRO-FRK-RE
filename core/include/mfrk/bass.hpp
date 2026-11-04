#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace mfrk {

struct BassState {
    // Explicit inputs: original inline constructor leaves words/dirty unwritten.
    std::array<std::uint16_t, 3> words;
    std::array<std::uint8_t, 3> dirty;
    float period, frequency = 0;
    std::array<float, 3> target{}, smoothed{};
    float phase_a = 0, phase_b = 0.25f, phase_seed = 0;
    float copied_frequency = 0;
    std::uint32_t random = 1, second_random = 2;
    std::uint8_t trigger_suppressed = 1;
    float level;

    static BassState admitted(std::array<std::uint16_t, 3> words,
                              std::array<std::uint8_t, 3> dirty) noexcept;
};

// Exact native32-frame Bass at96kHz; no table or math-import dependency.
// Finite state; render/trigger phase-to-int32 truncations must be representable.
class Bass {
    BassState s_;

public:
    explicit Bass(BassState admitted) noexcept : s_(admitted) {}
    const BassState& state() const noexcept { return s_; }

    void set_inputs(std::array<std::uint16_t, 3> words,
                    std::array<std::uint8_t, 3> dirty) noexcept {
        s_.words = words;
        s_.dirty = dirty;
    }
    void set_pitch(float frequency_hz) noexcept { s_.frequency = frequency_hz; }
    void set_phase(float phase) noexcept;
    void trigger(std::uint8_t flag) noexcept;
    // Native clears only3target controls; preserves phases/smoothed controls.
    void initialize() noexcept;
    void prepare_controls(bool clear_dirty = false) noexcept;
    void render_raw(std::span<float, 32> output) noexcept;
    void render_scaled(std::span<float, 32> output) noexcept;
};

}  // namespace mfrk
