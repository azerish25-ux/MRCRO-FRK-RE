#include "mfrk/filter_resource.hpp"
#include "mfrk/fir_resampler.hpp"
#include "mfrk/iir_resampler.hpp"

#include <cassert>
#include <limits>
#include <stdexcept>
#include <variant>
#include <vector>

namespace mfrk {
namespace {
using Resampler = std::variant<IirResampler, FirResampler>;
Resampler make_resampler(FilterOversampling factor, FilterKernel kernel, std::size_t frames) {
    switch (kernel) {
    case FilterKernel::iir: return Resampler(std::in_place_type<IirResampler>, factor);
    case FilterKernel::fir: return Resampler(std::in_place_type<FirResampler>, factor, frames);
    }
    throw std::invalid_argument("Unsupported original filter kernel");
}
std::size_t checked_capacity(std::size_t frames, std::size_t factor) {
    if (frames == 0 || frames > std::numeric_limits<std::size_t>::max() / factor) throw std::length_error("Invalid filter capacity");
    return frames * factor;
}
}  // namespace

struct FilterResource::Impl {
    Resampler audio, modulation;
    std::size_t factor, maximum_frames;
    std::vector<float> up_audio, up_modulation, raw_output;
    Impl(FilterOversampling ratio, FilterKernel kernel, std::size_t frames)
        : audio(make_resampler(ratio, kernel, frames)), modulation(make_resampler(ratio, kernel, frames)),
          factor(static_cast<std::size_t>(ratio)), maximum_frames(frames),
          up_audio(checked_capacity(frames, factor)), up_modulation(up_audio.size()), raw_output(up_audio.size()) {}
};

FilterResource::FilterResource(FilterOversampling factor, FilterKernel kernel, std::size_t frames)
    : impl_(std::make_unique<Impl>(factor, kernel, frames)) {}
FilterResource::~FilterResource() = default;
void FilterResource::reset() noexcept {
    std::visit([](auto& resampler) { resampler.reset(); }, impl_->audio);
    std::visit([](auto& resampler) { resampler.reset(); }, impl_->modulation);
}
void FilterResource::process(Vcf49& filter, std::span<const float> audio,
                             std::span<const float> modulation, std::span<float> output) noexcept {
    assert(audio.size() == modulation.size() && audio.size() == output.size() && audio.size() <= impl_->maximum_frames);
    const auto frames = audio.size() * impl_->factor;
    auto up_audio = std::span<float>(impl_->up_audio).first(frames);
    auto up_modulation = std::span<float>(impl_->up_modulation).first(frames);
    auto raw_output = std::span<float>(impl_->raw_output).first(frames);
    std::visit([&](auto& resampler) { resampler.upsample(audio, up_audio); }, impl_->audio);
    std::visit([&](auto& resampler) { resampler.upsample(modulation, up_modulation); }, impl_->modulation);
    filter.process(up_audio, up_modulation, raw_output);
    std::visit([&](auto& resampler) { resampler.downsample(raw_output, output); }, impl_->audio);
}
std::size_t FilterResource::factor() const noexcept { return impl_->factor; }
std::uint32_t FilterResource::latency_frames() const noexcept {
    return std::visit([](const auto& resampler) { return resampler.latency_frames(); }, impl_->audio);
}
}  // namespace mfrk
