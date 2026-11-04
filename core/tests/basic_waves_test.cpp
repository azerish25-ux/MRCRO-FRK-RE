#include "mfrk/basic_waves.hpp"
#include "basic_waves_vectors.hpp"

#include <array>
#include <bit>
#include <cstdio>

int main() {
    const mfrk::BasicWaveTables tables;
    std::size_t samples = 0;
    std::size_t failures = 0;
    for (const auto& scenario : kBasicScenarios) {
        mfrk::BasicWaves engine(tables);
        engine.set_pitch(std::bit_cast<float>(scenario.frequency), std::bit_cast<float>(scenario.table_key));
        engine.set_phase_seed(std::bit_cast<float>(scenario.phase));
        if (scenario.warm_controls) engine.prepare_controls(scenario.initial_words, {1, 1, 1});
        engine.reset();
        std::size_t case_failures = 0;
        for (const auto& block : scenario.blocks) {
            if (block.set_pitch) engine.set_pitch(std::bit_cast<float>(block.frequency), std::bit_cast<float>(block.table_key));
            engine.prepare_controls(block.words, block.dirty);
            std::array<float, 32> output{};
            engine.render(output);
            for (std::size_t index = 0; index < output.size(); ++index) {
                const auto actual = std::bit_cast<std::uint32_t>(output[index]);
                if (actual != block.output[index]) {
                    if (case_failures < 2) std::printf("%s sample %zu: expected %08x, got %08x\n", scenario.name, index, block.output[index], actual);
                    ++case_failures;
                    ++failures;
                }
                ++samples;
            }
        }
        std::printf("%s: %zu mismatches\n", scenario.name, case_failures);
    }
    std::printf("Original-instruction Basic Waves: %zu samples, %zu mismatches\n", samples, failures);
    return failures == 0 ? 0 : 1;
}
