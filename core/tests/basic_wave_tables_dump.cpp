#include "mfrk/basic_wave_tables.hpp"

#include <cstdio>
#include <fstream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "Usage: mfrk_basic_tables_dump OUTPUT\n");
        return 2;
    }
    const mfrk::BasicWaveTables tables;
    std::ofstream output(argv[1], std::ios::binary | std::ios::trunc);
    if (!output) return 2;
    for (std::uint32_t row = 0; row < 4; ++row) {
        for (std::uint32_t bank = 0; bank < 32; ++bank) {
            const auto& table = tables.lookup(static_cast<mfrk::BasicWaveform>(row), bank);
            for (const auto sample : table) {
                const auto bits = static_cast<std::uint16_t>(sample);
                const char bytes[] = {static_cast<char>(bits & 0xffu), static_cast<char>(bits >> 8)};
                output.write(bytes, sizeof(bytes));
            }
        }
    }
    output.close();
    if (!output) return 2;
    std::printf("Generated 128 Basic Waves bank selections as little-endian signed-short bytes\n");
    return 0;
}
