#include "mfrk/vcf49.hpp"

#include <bit>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace {
std::uint32_t word(std::ifstream& stream) {
    unsigned char bytes[4];
    if (!stream.read(reinterpret_cast<char*>(bytes), 4)) throw std::runtime_error("Truncated VCF49 fixture");
    return std::uint32_t{bytes[0]} | (std::uint32_t{bytes[1]} << 8) |
        (std::uint32_t{bytes[2]} << 16) | (std::uint32_t{bytes[3]} << 24);
}
float scalar(std::ifstream& stream) { return std::bit_cast<float>(word(stream)); }
mfrk::VcfControls controls(std::ifstream& stream) {
    mfrk::VcfControls result{};
    result.cutoff = scalar(stream);
    result.resonance = scalar(stream);
    result.output = static_cast<mfrk::VcfOutput>(word(stream));
    result.noise = word(stream) != 0;
    for (auto& flag : result.dirty) flag = static_cast<std::uint8_t>(word(stream));
    result.first = word(stream) != 0;
    result.force = word(stream) != 0;
    return result;
}
}  // namespace

int main(int argc, char** argv) try {
    if (argc != 2) throw std::runtime_error("Expected original VCF49 fixture path");
    std::ifstream stream(argv[1], std::ios::binary);
    if (!stream || word(stream) != 0x3439464du || word(stream) != 1) throw std::runtime_error("Bad fixture header");
    const auto cases = word(stream);
    if (cases == 0 || cases > 1000) throw std::runtime_error("Bad case count");
    std::size_t failures = 0, samples = 0, blocks = 0;
    for (std::uint32_t c = 0; c < cases; ++c) {
        const float rate = scalar(stream);
        const auto seed = word(stream);
        mfrk::Vcf49 filter(rate, seed);
        auto state = controls(stream);
        filter.prepare(state);
        filter.reset(state.resonance);
        const auto count = word(stream);
        if (count == 0 || count > 1000) throw std::runtime_error("Bad block count");
        for (std::uint32_t b = 0; b < count; ++b) {
            state = controls(stream);
            const bool reset = word(stream) != 0;
            const auto frames = word(stream), expected_seed = word(stream);
            if (frames == 0 || frames > 4096) throw std::runtime_error("Bad frame count");
            filter.prepare(state);
            if (reset) filter.reset(state.resonance);
            std::vector<float> input(frames), modulation(frames), output(frames);
            std::vector<std::uint32_t> expected(frames);
            for (auto& value : input) value = scalar(stream);
            for (auto& value : modulation) value = scalar(stream);
            for (auto& value : expected) value = word(stream);
            filter.process(input, modulation, output);
            for (std::size_t i = 0; i < frames; ++i) {
                const auto actual = std::bit_cast<std::uint32_t>(output[i]);
                if (actual != expected[i]) {
                    if (failures < 4) std::printf("Case %u block %u sample %zu: %08x != %08x\n", c,b,i,actual,expected[i]);
                    ++failures;
                }
                ++samples;
            }
            if (filter.noise_state() != expected_seed) ++failures;
            ++blocks;
        }
    }
    if (stream.peek() != std::char_traits<char>::eof()) throw std::runtime_error("Trailing fixture bytes");
    std::printf("Original-instruction VCF49: %zu samples, %zu blocks, %zu mismatches\n", samples, blocks, failures);
    return failures == 0 ? 0 : 1;
} catch (const std::exception& error) {
    std::fprintf(stderr, "%s\n", error.what());
    return 2;
}
