#include "mfrk/basic_waves.hpp"

#include <algorithm>

namespace mfrk {

struct BasicWaves::Impl {};
BasicWaves::BasicWaves(const BasicWaveTables&) : impl_(std::make_unique<Impl>()) {}
BasicWaves::~BasicWaves() = default;
void BasicWaves::set_pitch(float, float) noexcept {}
void BasicWaves::set_phase_seed(float) noexcept {}
void BasicWaves::reset() noexcept {}
void BasicWaves::prepare_controls(const std::array<std::uint16_t, 3>&,
                                  const std::array<std::uint8_t, 3>&) noexcept {}
// RED phase: populated original-reference traces must reject silence.
void BasicWaves::render(std::span<float, 32> output) noexcept {
    std::fill(output.begin(), output.end(), 0.0f);
}

}  // namespace mfrk
