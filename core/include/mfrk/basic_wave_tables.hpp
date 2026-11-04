#pragma once

#include <array>
#include <cstdint>

namespace mfrk {

enum class BasicWaveform : std::uint8_t { saw, square, triangle, sine };

// Construct before audio processing. Tables are immutable afterward and should
// be shared by oscillator instances. The bank index must be in [0, 31].
class BasicWaveTables {
public:
    using Table = std::array<std::int16_t, 512>;
    BasicWaveTables() noexcept;
    [[nodiscard]] const Table& lookup(BasicWaveform waveform, std::uint32_t bank) const noexcept;

private:
    std::array<Table, 34> tables_{};
};

}  // namespace mfrk
