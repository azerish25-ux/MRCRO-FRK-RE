#include "mfrk/basic_wave_tables.hpp"
#include "basic_wave_table_map.hpp"

#include <cassert>
#include <cmath>

namespace mfrk {

namespace {

double harmonic_coefficient(BasicWaveform waveform, unsigned harmonic) noexcept {
    const double n = static_cast<double>(harmonic);
    switch (waveform) {
    case BasicWaveform::saw:
        return (1.0 / n) * ((harmonic & 1u) ? 0.6366197723675814 : -0.6366197723675814);
    case BasicWaveform::square:
        return (harmonic & 1u) ? 1.2732395447351628 / n : 0.0;
    case BasicWaveform::triangle:
        if ((harmonic & 1u) == 0) return 0.0;
        return (1.0 / (n * n)) * ((harmonic & 2u) ? -0.8105694691387022 : 0.8105694691387022);
    case BasicWaveform::sine:
        return 0.0;
    }
    return 0.0;
}

}  // namespace

BasicWaveTables::BasicWaveTables() noexcept {
    constexpr double angle_step = 0.006135923151542565;
    for (std::size_t index = 0; index < tables_.size(); ++index) {
        const auto definition = detail::kBasicWaveDefinitions[index];
        for (std::size_t sample = 0; sample < tables_[index].size(); ++sample) {
            double value;
            if (definition.waveform == BasicWaveform::sine) {
                value = ((std::sin(static_cast<double>(2 * sample) * angle_step) * 32767.0) * 0.8) + 0.5;
            } else {
                double sum = 0.0;
                for (unsigned harmonic = 1; harmonic <= definition.harmonics; ++harmonic) {
                    const double angle = (static_cast<double>(harmonic) * static_cast<double>(2 * sample)) * angle_step;
                    sum = sum + harmonic_coefficient(definition.waveform, harmonic) * std::sin(angle);
                }
                value = sum * 26213.600000000002 + 0.5;
            }
            // Original conversion truncates after +0.5, including negative values.
            tables_[index][sample] = static_cast<std::int16_t>(static_cast<std::int32_t>(value));
        }
    }
}

const BasicWaveTables::Table& BasicWaveTables::lookup(BasicWaveform waveform,
                                                    std::uint32_t bank) const noexcept {
    const auto row = static_cast<std::size_t>(waveform);
    assert(row < detail::kBasicWaveBankMap.size() && bank < 32);
    return tables_[detail::kBasicWaveBankMap[row][bank]];
}

}  // namespace mfrk
