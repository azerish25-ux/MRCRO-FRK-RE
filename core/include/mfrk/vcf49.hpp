#pragma once

#include "mfrk/vcf_types.hpp"

#include <memory>
#include <span>

namespace mfrk {

// Raw model49 at its actual internal rate; filter oversampling/dual selection
// are separate stages. Finite normalized controls0..1, positive rate, explicit
// PRNG seed. Prepare normalized records even when dirty flags are clear.
class Vcf49 {
public:
    Vcf49(float sample_rate, std::uint32_t noise_seed);
    ~Vcf49();
    Vcf49(const Vcf49&) = delete;
    Vcf49& operator=(const Vcf49&) = delete;
    void prepare(const VcfControls& controls) noexcept;
    void reset(float resonance) noexcept;
    // Worker-owned generation of the same logical filter after public Init.
    // Preserve latched controls and PRNG; original Init/reset clears DSP history.
    [[nodiscard]] std::unique_ptr<Vcf49> reconfigured(float sample_rate, float reset_resonance) const;
    // Equal span lengths; finite audio/modulation. Exact in-place output over
    // an input is supported; partially overlapping ranges are not supported.
    void process(std::span<const float> audio, std::span<const float> modulation,
                 std::span<float> output) noexcept;
    [[nodiscard]] std::uint32_t noise_state() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace mfrk
