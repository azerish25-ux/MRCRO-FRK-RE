// Exact mathematical table descriptors and routing recovered from the matched processor.
#pragma once
#include <array>
#include <cstdint>
namespace mfrk::detail {
struct BasicWaveDefinition { BasicWaveform waveform; std::uint16_t harmonics; };
inline constexpr std::array<BasicWaveDefinition,34> kBasicWaveDefinitions = {{
    {BasicWaveform::saw, 256},
    {BasicWaveform::saw, 192},
    {BasicWaveform::saw, 153},
    {BasicWaveform::saw, 121},
    {BasicWaveform::saw, 96},
    {BasicWaveform::saw, 76},
    {BasicWaveform::saw, 60},
    {BasicWaveform::saw, 48},
    {BasicWaveform::saw, 38},
    {BasicWaveform::saw, 24},
    {BasicWaveform::saw, 15},
    {BasicWaveform::saw, 9},
    {BasicWaveform::saw, 3},
    {BasicWaveform::square, 256},
    {BasicWaveform::square, 192},
    {BasicWaveform::square, 153},
    {BasicWaveform::square, 121},
    {BasicWaveform::square, 96},
    {BasicWaveform::square, 76},
    {BasicWaveform::square, 60},
    {BasicWaveform::square, 48},
    {BasicWaveform::square, 38},
    {BasicWaveform::square, 24},
    {BasicWaveform::square, 15},
    {BasicWaveform::square, 9},
    {BasicWaveform::square, 3},
    {BasicWaveform::triangle, 256},
    {BasicWaveform::triangle, 192},
    {BasicWaveform::triangle, 121},
    {BasicWaveform::triangle, 76},
    {BasicWaveform::triangle, 48},
    {BasicWaveform::triangle, 24},
    {BasicWaveform::triangle, 9},
    {BasicWaveform::sine, 0},
}};
inline constexpr std::array<std::array<std::uint8_t,32>,4> kBasicWaveBankMap = {{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 10, 10, 11, 11, 12, 12, 12, 12, 12, 12},
    {13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 22, 23, 23, 24, 24, 25, 25, 25, 25, 25, 25},
    {26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 28, 28, 29, 29, 30, 30, 31, 31, 31, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33},
    {33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33},
}};
}  // namespace mfrk::detail
