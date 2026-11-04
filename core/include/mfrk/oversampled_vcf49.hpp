#pragma once

#include "mfrk/iir_resampler.hpp"
#include "mfrk/vcf49.hpp"

#include <vector>

namespace mfrk {

// Original settled fixed-resource chain. The outer asynchronous resource/fade
// lifecycle and dual-model selector remain separate. Construct off audio thread.
class OversampledVcf49 {
public:
    OversampledVcf49(float host_rate, FilterOversampling factor,
                     std::size_t maximum_frames, std::uint32_t seed);
    void prepare(const VcfControls& controls) noexcept;
    void reset(float resonance) noexcept;
    void process(std::span<const float> audio, std::span<const float> modulation,
                 std::span<float> output) noexcept;
    [[nodiscard]] std::uint32_t noise_state() const noexcept;

private:
    IirResampler audio_resampler_, modulation_resampler_;
    Vcf49 filter_;
    std::size_t maximum_frames_;
    std::vector<float> up_audio_, up_modulation_, raw_output_;
};

}  // namespace mfrk
