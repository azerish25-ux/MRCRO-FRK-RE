#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <span>

namespace mfrk {

enum class VcfOutput : std::uint8_t { lowpass, bandpass, highpass };

struct Vcf37Controls {
    float cutoff;
    float resonance;
    VcfOutput output;
    bool noise;
    std::array<std::uint8_t, 4> dirty;
    bool first;
    bool force;
};

struct Vcf37ControlSnapshot {
    float cutoff_hz;
    float g;
    float r;
    double g_target;
    double r_target;
};

// Raw hardware-model37 stage, not the dual/model49/oversampling wrapper. Controls
// are normalized finite0..1; rate is positive; source lifecycle flags and initial
// PRNG state are explicitly caller-supplied. Construct outside the audio callback.
class Vcf37 {
public:
    Vcf37(float sample_rate, std::uint32_t noise_seed);
    ~Vcf37();
    Vcf37(const Vcf37&) = delete;
    Vcf37& operator=(const Vcf37&) = delete;

    void prepare(const Vcf37Controls& controls) noexcept;
    void reset(float resonance) noexcept;
    void process(std::span<const float, 32> input, std::span<float, 32> output) noexcept;
    [[nodiscard]] Vcf37ControlSnapshot control_snapshot() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace mfrk
