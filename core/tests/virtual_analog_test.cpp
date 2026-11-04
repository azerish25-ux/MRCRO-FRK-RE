#include "mfrk/virtual_analog.hpp"
#include "virtual_analog_vectors.hpp"

#include <array>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cmath>

int main() {
    std::size_t samples = 0;
    std::size_t failures = 0;
    for (const auto& scenario : kVAnalogScenarios) {
        mfrk::VirtualAnalog engine;
        std::size_t case_failures = 0;
        std::size_t block_index = 0;
        double max_absolute_difference = 0.0;
        for (const auto& block : scenario.blocks) {
            const auto& bits = block.parameters;
            const mfrk::VirtualAnalogParameters parameters{
                std::bit_cast<float>(bits[0]), std::bit_cast<float>(bits[1]),
                std::bit_cast<float>(bits[2]), std::bit_cast<float>(bits[3])};
            std::array<float, 32> output{};
            engine.render(parameters, output);
            for (std::size_t index = 0; index < output.size(); ++index) {
                const auto actual = std::bit_cast<std::uint32_t>(output[index]);
                if (actual != block.output[index]) {
                    if (case_failures < 2) {
                        std::printf("%s block %zu sample %zu: expected 0x%08x, got 0x%08x\n",
                                    scenario.name, block_index, index, block.output[index], actual);
                    }
                    const auto expected = std::bit_cast<float>(block.output[index]);
                    max_absolute_difference = std::max(max_absolute_difference,
                        std::abs(static_cast<double>(output[index]) - expected));
                    ++case_failures;
                    ++failures;
                }
                ++samples;
            }
            ++block_index;
        }
        std::printf("%s: %zu mismatches; max absolute difference %.12g\n",
                    scenario.name, case_failures, max_absolute_difference);
    }
    std::printf("Original-instruction VAnalog vectors: %zu checked, %zu mismatches\n",
                samples, failures);
    return failures == 0 ? 0 : 1;
}
