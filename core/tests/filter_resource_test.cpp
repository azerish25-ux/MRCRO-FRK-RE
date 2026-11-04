#include "mfrk/filter_resampling.hpp"
#include "mfrk/fir_resampler.hpp"
#include "mfrk/iir_resampler.hpp"

#include <bit>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace {
struct Reference {
    std::ifstream file;
    explicit Reference(const char* path) : file(path, std::ios::binary) {
        if (!file) throw std::runtime_error("Cannot open numeric filter fixture");
    }
    std::uint32_t word() {
        unsigned char b[4];
        if (!file.read(reinterpret_cast<char*>(b),4)) throw std::runtime_error("Truncated numeric filter fixture");
        return std::uint32_t{b[0]} | (std::uint32_t{b[1]} << 8) | (std::uint32_t{b[2]} << 16) | (std::uint32_t{b[3]} << 24);
    }
    float scalar() { return std::bit_cast<float>(word()); }
    std::uint32_t count() {
        const auto result = word();
        if (result == 0 || result > 4096) throw std::runtime_error("Invalid numeric filter count");
        return result;
    }
    std::vector<float> samples(std::size_t frames) {
        std::vector<float> values(frames);
        for (auto& value : values) value = scalar();
        return values;
    }
    std::size_t compare(std::span<const float> actual) {
        std::size_t failures = 0;
        for (float value : actual) if (std::bit_cast<std::uint32_t>(value) != word()) ++failures;
        return failures;
    }
};
}  // namespace

int main(int argc, char** argv) try {
    if (argc != 2) throw std::runtime_error("Expected numeric filter fixture path");
    Reference reference(argv[1]);
    if (reference.word() != 0x5246534D || reference.word() != 1) throw std::runtime_error("Invalid numeric filter header");
    std::size_t failures = 0, up_samples = 0, down_samples = 0, fade_samples = 0;
    const auto fir_cases = reference.count();
    for (std::uint32_t c = 0; c < fir_cases; ++c) {
        const auto factor = static_cast<mfrk::FilterOversampling>(reference.word());
        mfrk::FirResampler resampler(factor,32);
        if (resampler.latency_frames() != reference.word()) ++failures;
        const auto blocks = reference.count();
        for (std::uint32_t b = 0; b < blocks; ++b) {
            const bool reset = reference.word() != 0;
            const auto frames = reference.count();
            if (frames > 32) throw std::runtime_error("FIR fixture exceeds declared capacity");
            if (reset) resampler.reset();
            auto input = reference.samples(frames);
            std::vector<float> up(frames * resampler.factor()), down(frames);
            resampler.upsample(input,up); resampler.downsample(up,down);
            failures += reference.compare(up); failures += reference.compare(down);
            up_samples += up.size(); down_samples += down.size();
        }
    }
    const auto fallback_cases = reference.count();
    std::size_t fallback_up = 0, fallback_down = 0, fallback_states = 0;
    for (std::uint32_t c = 0; c < fallback_cases; ++c) {
        const auto factor = static_cast<mfrk::FilterOversampling>(reference.word());
        mfrk::IirResampler resampler(factor);
        if (resampler.latency_frames() != reference.word()) ++failures;
        const auto states = reference.count(), blocks = reference.count();
        if (states > 16) throw std::runtime_error("Fallback state exceeds capacity");
        for (std::uint32_t b = 0; b < blocks; ++b) {
            const bool reset = reference.word() != 0;
            const auto frames = reference.count();
            if (reset) resampler.reset();
            auto input = reference.samples(frames);
            std::vector<float> up(frames * resampler.factor()), down(frames);
            resampler.upsample(input,up); resampler.downsample(up,down);
            failures += reference.compare(up); failures += reference.compare(down);
            const auto state = resampler.state();
            failures += reference.compare(std::span<const float>(state.up).first(states));
            failures += reference.compare(std::span<const float>(state.down).first(states));
            fallback_up += up.size(); fallback_down += down.size(); ++fallback_states;
        }
    }
    const auto fade_cases = reference.count();
    for (std::uint32_t c = 0; c < fade_cases; ++c) {
        const float rate = reference.scalar(), time = reference.scalar();
        const float current = reference.scalar(), target = reference.scalar();
        mfrk::FilterFade fade(rate,time,1.0f,1,current);
        fade.set_target(target);
        if (std::bit_cast<std::uint32_t>(fade.state().step) != reference.word()) ++failures;
        const auto blocks = reference.count();
        for (std::uint32_t b = 0; b < blocks; ++b) {
            std::array<float,32> values;
            fade.render(values);
            failures += reference.compare(values);
            if (std::bit_cast<std::uint32_t>(fade.state().current) != reference.word()) ++failures;
            fade_samples += values.size();
        }
    }
    const auto selector_cases = reference.count();
    for (std::uint32_t c = 0; c < selector_cases; ++c) {
        const auto index = reference.word(); const bool adaptive = reference.word() != 0;
        const float rate = reference.scalar(); const auto expected = reference.word();
        if (static_cast<std::uint32_t>(mfrk::select_filter_factor(index,adaptive,rate)) != expected) ++failures;
    }
    if (reference.file.peek() != std::char_traits<char>::eof()) throw std::runtime_error("Trailing numeric filter bytes");
    std::printf("Original FIR: %zu up, %zu down; fade %zu samples; selector %u cases; %zu mismatches\n",
                up_samples,down_samples,fade_samples,selector_cases,failures);
    std::printf("Original IIR fallback: %zu up, %zu down, %zu paired states\n",fallback_up,fallback_down,fallback_states);
    return failures == 0 ? 0 : 1;
} catch (const std::exception& error) { std::fprintf(stderr,"%s\n",error.what()); return 2; }
