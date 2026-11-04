#include "mfrk/virtual_analog_oscillator.hpp"
#include "virtual_analog_oscillator_vectors.hpp"

#include <array>
#include <bit>
#include <cstdio>

int main() {
    std::size_t samples = 0;
    std::size_t failures = 0;
    for (const auto& scenario : kOutputScenarios) {
        mfrk::VirtualAnalogOscillator oscillator(std::bit_cast<float>(scenario.initial_previous_bits));
        for (const auto& block : scenario.blocks) {
            const auto& bits = block.parameters;
            const mfrk::VirtualAnalogParameters parameters{
                std::bit_cast<float>(bits[0]), std::bit_cast<float>(bits[1]),
                std::bit_cast<float>(bits[2]), std::bit_cast<float>(bits[3])};
            std::array<float, 32> output{};
            oscillator.render(parameters, std::bit_cast<float>(block.level), output);
            for (std::size_t index = 0; index < output.size(); ++index) {
                const auto actual = std::bit_cast<std::uint32_t>(output[index]);
                if (actual != block.output[index]) {
                    if (failures < 8) {
                        std::printf("%s sample %zu: expected %08x, got %08x\n",
                                    scenario.name, index, block.output[index], actual);
                    }
                    ++failures;
                }
                ++samples;
            }
        }
    }
    std::printf("Original-instruction VAnalog output wrapper: %zu samples, %zu mismatches\n", samples, failures);
    return failures == 0 ? 0 : 1;
}
