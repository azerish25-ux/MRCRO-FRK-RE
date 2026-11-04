#include "mfrk/pitch.hpp"
#include "pitch_vectors.hpp"

#include <bit>
#include <cstdint>
#include <cstdio>

int main() {
    std::size_t failures = 0;
    for (const auto& vector : kPitchVectors) {
        const auto note = std::bit_cast<float>(vector.input_bits);
        const auto actual = std::bit_cast<std::uint32_t>(mfrk::normalized_pitch_frequency(note));
        if (actual != vector.output_bits) {
            if (failures < 10) {
                std::printf("note %.9g: expected 0x%08x, got 0x%08x\n",
                            static_cast<double>(note), vector.output_bits, actual);
            }
            ++failures;
        }
    }
    std::printf("Original-instruction pitch vectors: %zu checked, %zu mismatches\n",
                kPitchVectors.size(), failures);
    return failures == 0 ? 0 : 1;
}
