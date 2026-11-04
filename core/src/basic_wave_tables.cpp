#include "mfrk/basic_wave_tables.hpp"
#include "basic_wave_table_map.hpp"

#include <cassert>

namespace mfrk {

// RED phase: byte hashes from independently observed original initialized tables
// must reject zero-filled tables before the recovered generation is implemented.
BasicWaveTables::BasicWaveTables() noexcept = default;

const BasicWaveTables::Table& BasicWaveTables::lookup(BasicWaveform waveform,
                                                    std::uint32_t bank) const noexcept {
    const auto row = static_cast<std::size_t>(waveform);
    assert(row < detail::kBasicWaveBankMap.size() && bank < 32);
    return tables_[detail::kBasicWaveBankMap[row][bank]];
}

}  // namespace mfrk
