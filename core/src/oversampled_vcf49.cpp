#include "mfrk/oversampled_vcf49.hpp"
#include <cassert>
#include <limits>
#include <stdexcept>

namespace mfrk {
namespace {
std::size_t buffer_frames(std::size_t frames, std::size_t factor) {
    if (frames == 0 || frames > std::numeric_limits<std::size_t>::max() / factor) {
        throw std::length_error("Invalid original filter block capacity");
    }
    return frames * factor;
}
}  // namespace

OversampledVcf49::OversampledVcf49(float rate, FilterOversampling factor,
                                 std::size_t frames, std::uint32_t seed)
    : audio_resampler_(factor), modulation_resampler_(factor),
      filter_(rate * static_cast<float>(factor), seed), maximum_frames_(frames),
      up_audio_(buffer_frames(frames, audio_resampler_.factor())),
      up_modulation_(up_audio_.size()), raw_output_(up_audio_.size()) {}

void OversampledVcf49::prepare(const VcfControls& controls) noexcept { filter_.prepare(controls); }

void OversampledVcf49::reset(float resonance) noexcept {
    audio_resampler_.reset();
    modulation_resampler_.reset();
    filter_.reset(resonance);
}

void OversampledVcf49::process(std::span<const float> audio, std::span<const float> modulation,
                              std::span<float> output) noexcept {
    assert(audio.size() == modulation.size() && audio.size() == output.size());
    assert(audio.size() <= maximum_frames_);
    const auto frames = audio.size() * audio_resampler_.factor();
    auto up_audio = std::span<float>(up_audio_).first(frames);
    auto up_modulation = std::span<float>(up_modulation_).first(frames);
    auto raw_output = std::span<float>(raw_output_).first(frames);
    audio_resampler_.upsample(audio, up_audio);
    modulation_resampler_.upsample(modulation, up_modulation);
    filter_.process(up_audio, up_modulation, raw_output);
    audio_resampler_.downsample(raw_output, output);
}

std::uint32_t OversampledVcf49::noise_state() const noexcept { return filter_.noise_state(); }
}  // namespace mfrk
