#include "mfrk/vcf49.hpp"

#include <algorithm>

namespace mfrk {
struct Vcf49::Impl {};
Vcf49::Vcf49(float, std::uint32_t) : impl_(std::make_unique<Impl>()) {}
Vcf49::~Vcf49() = default;
void Vcf49::prepare(const VcfControls&) noexcept {}
void Vcf49::reset(float) noexcept {}
void Vcf49::process(std::span<const float>, std::span<const float>, std::span<float> output) noexcept {
    std::fill(output.begin(), output.end(), 0.0f);
}
std::uint32_t Vcf49::noise_state() const noexcept { return 0; }
}  // namespace mfrk
