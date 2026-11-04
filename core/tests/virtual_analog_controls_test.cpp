#include "mfrk/virtual_analog_controls.hpp"
#include "virtual_analog_controls_vectors.hpp"

#include <array>
#include <bit>
#include <cstdio>

int main() {
    std::size_t updates = 0;
    std::size_t failures = 0;
    for (const auto& scenario : kControlScenarios) {
        mfrk::VirtualAnalogControlState state;
        state.latched = scenario.initial_latched;
        state.trigger = scenario.initial_trigger;
        const auto& initial = scenario.initial_parameters;
        state.parameters = {std::bit_cast<float>(initial[0]), std::bit_cast<float>(initial[1]),
                            std::bit_cast<float>(initial[2]), std::bit_cast<float>(initial[3])};
        for (const auto& step : scenario.steps) {
            state.words = step.words;
            state.dirty = step.dirty;
            state.signal = step.signal;
            mfrk::update_virtual_analog_controls(state);
            const std::array<std::uint32_t, 4> actual{
                std::bit_cast<std::uint32_t>(state.parameters.note),
                std::bit_cast<std::uint32_t>(state.parameters.timbre),
                std::bit_cast<std::uint32_t>(state.parameters.morph),
                std::bit_cast<std::uint32_t>(state.parameters.harmonics)};
            if (actual != step.parameters || state.trigger != step.trigger ||
                state.signal != step.signal_after || state.latched != step.latched_after ||
                state.words != step.words_after || state.dirty != step.dirty_after) {
                if (failures < 8) {
                    std::printf("%s update %zu: control state mismatch\n", scenario.name, updates);
                }
                ++failures;
            }
            ++updates;
        }
    }
    std::printf("Original-instruction VAnalog controls: %zu updates, %zu mismatches\n", updates, failures);
    return failures == 0 ? 0 : 1;
}
