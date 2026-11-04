#include "mfrk/vcf37.hpp"

#include <algorithm>

namespace mfrk {

struct Vcf37::Impl {};
Vcf37::Vcf37(float, std::uint32_t) : impl_(std::make_unique<Impl>()) {}
Vcf37::~Vcf37() = default;
void Vcf37::prepare(const Vcf37Controls&) noexcept {}
void Vcf37::reset(float) noexcept {}
// RED phase: original nonzero signals and original prepared coefficients reject this.
void Vcf37::process(std::span<const float, 32>, std::span<float, 32> output) noexcept {
    std::fill(output.begin(), output.end(), 0.0f);
}
Vcf37ControlSnapshot Vcf37::control_snapshot() const noexcept { return {}; }

}  // namespace mfrk
