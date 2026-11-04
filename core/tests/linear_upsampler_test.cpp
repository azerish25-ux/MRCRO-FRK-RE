#include "mfrk/linear_upsampler.hpp"
#include "mfrk/virtual_analog.hpp"
#include "linear_upsampler_vectors.hpp"

#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>

int main() {
    std::size_t direct_samples = 0;
    std::size_t pipeline_samples = 0;
    std::size_t failures = 0;
    for (const auto& scenario : kUpsamplerScenarios) {
        const float previous = std::bit_cast<float>(scenario.initial_previous_bits);
        mfrk::LinearUpsampler2x unit(previous);
        mfrk::LinearUpsampler2x pipeline(previous);
        mfrk::VirtualAnalog oscillator;
        std::size_t case_failures = 0;
        for (const auto& block : scenario.blocks) {
            std::array<float, 16> input;
            for (std::size_t index = 0; index < input.size(); ++index) {
                input[index] = std::bit_cast<float>(block.input[index]);
            }
            std::array<float, 32> direct_output{};
            unit.process(input, direct_output);
            std::array<float, 32> pipeline_output{};
            if (scenario.vanalog_pipeline) {
                const auto& bits = block.parameters;
                const mfrk::VirtualAnalogParameters parameters{
                    std::bit_cast<float>(bits[0]), std::bit_cast<float>(bits[1]),
                    std::bit_cast<float>(bits[2]), std::bit_cast<float>(bits[3])};
                std::array<float, 16> raw{};
                oscillator.render(parameters, raw);
                pipeline.process(raw, pipeline_output);
            }
            for (std::size_t index = 0; index < direct_output.size(); ++index) {
                const auto actual = std::bit_cast<std::uint32_t>(direct_output[index]);
                if (actual != block.output[index]) {
                    if (case_failures < 2) {
                        std::printf("%s direct sample %zu: expected %08x, got %08x\n",
                                    scenario.name, index, block.output[index], actual);
                    }
                    ++failures;
                    ++case_failures;
                }
                ++direct_samples;
                if (scenario.vanalog_pipeline) {
                    const auto composed = std::bit_cast<std::uint32_t>(pipeline_output[index]);
                    if (composed != block.output[index]) {
                        if (case_failures < 2) {
                            std::printf("%s pipeline sample %zu: expected %08x, got %08x\n",
                                        scenario.name, index, block.output[index], composed);
                        }
                        ++failures;
                        ++case_failures;
                    }
                    ++pipeline_samples;
                }
            }
        }
        std::printf("%s: %zu mismatches\n", scenario.name, case_failures);
    }
    std::printf("Original-instruction upsampler: %zu direct and %zu VAnalog-chain samples, %zu mismatches\n",
                direct_samples, pipeline_samples, failures);
    return failures == 0 ? 0 : 1;
}
