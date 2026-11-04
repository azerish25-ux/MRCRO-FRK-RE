#include "mfrk/vcf37.hpp"
#include "vcf37_vectors.hpp"

#include <array>
#include <bit>
#include <cstdio>

int main() {
    std::size_t samples = 0;
    std::size_t controls = 0;
    std::size_t failures = 0;
    for (const auto& scenario : kVcfScenarios) {
        mfrk::Vcf37 filter(std::bit_cast<float>(scenario.rate), scenario.seed);
        mfrk::Vcf37Controls state{std::bit_cast<float>(scenario.cutoff),
            std::bit_cast<float>(scenario.resonance), static_cast<mfrk::VcfOutput>(scenario.mode),
            false, {1, 1, 1, 1}, true, true};
        filter.prepare(state);
        filter.reset(state.resonance);
        state.first = state.force = false;
        std::size_t case_failures = 0;
        for (const auto& block : scenario.blocks) {
            if (block.prepare) {
                state.cutoff = std::bit_cast<float>(block.cutoff);
                state.resonance = std::bit_cast<float>(block.resonance);
                state.output = static_cast<mfrk::VcfOutput>(block.mode);
                state.dirty = block.dirty;
                filter.prepare(state);
            }
            std::array<float, 32> input;
            for (std::size_t i = 0; i < input.size(); ++i) input[i] = std::bit_cast<float>(block.input[i]);
            std::array<float, 32> output{};
            filter.process(input, output);
            for (std::size_t i = 0; i < output.size(); ++i) {
                const auto actual = std::bit_cast<std::uint32_t>(output[i]);
                if (actual != block.output[i]) {
                    if (case_failures < 2) std::printf("%s sample %zu: expected %08x got %08x\n", scenario.name, i, block.output[i], actual);
                    ++case_failures;
                    ++failures;
                }
                ++samples;
            }
            const auto snapshot = filter.control_snapshot();
            if (std::bit_cast<std::uint32_t>(snapshot.cutoff_hz) != block.hz ||
                std::bit_cast<std::uint32_t>(snapshot.g) != block.g ||
                std::bit_cast<std::uint32_t>(snapshot.r) != block.r ||
                std::bit_cast<std::uint64_t>(snapshot.g_target) != block.g_target ||
                std::bit_cast<std::uint64_t>(snapshot.r_target) != block.r_target) {
                if (case_failures < 2) std::printf("%s: control coefficient mismatch\n", scenario.name);
                ++case_failures;
                ++failures;
            }
            ++controls;
        }
        std::printf("%s: %zu mismatches\n", scenario.name, case_failures);
    }
    std::printf("Original-instruction VCF37: %zu samples, %zu control blocks, %zu mismatches\n", samples, controls, failures);
    return failures == 0 ? 0 : 1;
}
