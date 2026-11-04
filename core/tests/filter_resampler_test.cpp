#include "mfrk/iir_resampler.hpp"
#include "mfrk/oversampled_vcf49.hpp"

#include <bit>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace {
class Reference {
public:
    explicit Reference(const char* path) : file_(path, std::ios::binary) {
        if (!file_) throw std::runtime_error("Cannot open filter fixture");
    }
    std::uint32_t word() {
        unsigned char b[4];
        if (!file_.read(reinterpret_cast<char*>(b), 4)) throw std::runtime_error("Truncated filter fixture");
        return std::uint32_t{b[0]} | (std::uint32_t{b[1]} << 8) |
            (std::uint32_t{b[2]} << 16) | (std::uint32_t{b[3]} << 24);
    }
    float scalar() { return std::bit_cast<float>(word()); }
    std::uint32_t count() {
        const auto value = word();
        if (value == 0 || value > 4096) throw std::runtime_error("Bad filter fixture count");
        return value;
    }
    void header(std::uint32_t magic) {
        if (word() != magic || word() != 1) throw std::runtime_error("Bad filter fixture header");
    }
    std::vector<float> samples(std::size_t count) {
        std::vector<float> values(count);
        for (auto& value : values) value = scalar();
        return values;
    }
    std::size_t compare(std::span<const float> actual) {
        std::size_t failures = 0;
        for (float value : actual) if (std::bit_cast<std::uint32_t>(value) != word()) ++failures;
        return failures;
    }
    void finish() {
        if (file_.peek() != std::char_traits<char>::eof()) throw std::runtime_error("Trailing fixture bytes");
    }
private:
    std::ifstream file_;
};
}  // namespace

int main(int argc, char** argv) try {
    if (argc != 2) throw std::runtime_error("Expected filter fixture path");
    Reference reference(argv[1]);
    std::size_t failures = 0, up_samples = 0, down_samples = 0, state_blocks = 0, chain_samples = 0;
    reference.header(0x5249494D);
    const auto iir_cases = reference.count();
    for (std::uint32_t c = 0; c < iir_cases; ++c) {
        const auto factor = static_cast<mfrk::FilterOversampling>(reference.word());
        mfrk::IirResampler resampler(factor);
        if (resampler.latency_frames() != reference.word()) ++failures;
        const auto blocks = reference.count();
        for (std::uint32_t b = 0; b < blocks; ++b) {
            const bool reset = reference.word() != 0;
            const auto frames = reference.count();
            if (reset) resampler.reset();
            auto input = reference.samples(frames);
            std::vector<float> up(frames * resampler.factor()), down(frames);
            resampler.upsample(input, up);
            resampler.downsample(up, down);
            const auto state = resampler.state();
            failures += reference.compare(up);
            failures += reference.compare(down);
            failures += reference.compare(state.up);
            failures += reference.compare(state.down);
            up_samples += up.size(); down_samples += down.size(); ++state_blocks;
        }
    }
    std::printf("Original IIR: %zu up, %zu down, %zu paired states, %zu mismatches\n", up_samples, down_samples, state_blocks, failures);
    reference.header(0x43564F4D);
    const auto chain_cases = reference.count();
    for (std::uint32_t c = 0; c < chain_cases; ++c) {
        const auto factor = static_cast<mfrk::FilterOversampling>(reference.word());
        const float rate = reference.scalar();
        const auto seed = reference.word();
        mfrk::OversampledVcf49 filter(rate, factor, 32, seed);
        mfrk::VcfControls controls{};
        controls.cutoff = reference.scalar(); controls.resonance = reference.scalar();
        controls.output = static_cast<mfrk::VcfOutput>(reference.word());
        controls.dirty = {1,1,1,1}; controls.first = controls.force = true;
        filter.prepare(controls); filter.reset(controls.resonance);
        controls.first = controls.force = false;
        const auto blocks = reference.count();
        for (std::uint32_t b = 0; b < blocks; ++b) {
            controls.cutoff = reference.scalar(); controls.resonance = reference.scalar();
            controls.output = static_cast<mfrk::VcfOutput>(reference.word());
            controls.noise = reference.word() != 0;
            for (auto& flag : controls.dirty) flag = static_cast<std::uint8_t>(reference.word());
            const bool reset = reference.word() != 0;
            const auto frames = reference.count(), seed_after = reference.word();
            auto audio = reference.samples(frames), modulation = reference.samples(frames);
            std::vector<float> output(frames);
            filter.prepare(controls);
            if (reset) filter.reset(controls.resonance);
            filter.process(audio, modulation, output);
            failures += reference.compare(output);
            if (filter.noise_state() != seed_after) ++failures;
            chain_samples += frames;
        }
    }
    reference.finish();
    std::printf("Original settled filter chain: %zu samples, combined %zu mismatches\n", chain_samples, failures);
    return failures == 0 ? 0 : 1;
} catch (const std::exception& error) {
    std::fprintf(stderr, "%s\n", error.what()); return 2;
}
