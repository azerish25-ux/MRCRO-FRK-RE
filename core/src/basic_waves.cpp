#include "mfrk/basic_waves.hpp"

#include <algorithm>
#include <bit>
#include <cmath>

namespace mfrk {

namespace {

constexpr float from_bits(std::uint32_t bits) noexcept { return std::bit_cast<float>(bits); }

std::int16_t short_wrap(std::int32_t value) noexcept {
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(value));
}

struct Slew {
    float target = 0.0f;
    float current = from_bits(0x2b8cbcccu);
    // Preserve the constructor literal; recomputation differs by one ULP.
    float step = from_bits(0x3bda740eu);
    bool settled() const noexcept { return current == target; }
    void reset(float value) noexcept { target = current = value; }
    float next() noexcept {
        if (current != target) {
            const float delta = target - current;
            if (delta > step) current = step + current;
            else if (delta > -step) current = target;
            else current = current - step;
        }
        return current;
    }
};

struct LowPass {
    float target = 0.0f;
    float current = from_bits(0x2b8cbcccu);
    float coefficient = from_bits(0x3d064870u);
    float threshold = from_bits(0x2b8cbcccu);
    bool settled() const noexcept { return current == target; }
    void reset(float value) noexcept { target = current = value; }
    void set_target(float value) noexcept {
        if (target != value) {
            target = value;
            threshold = std::max(std::abs(value) * from_bits(0x2b8cbcccu), from_bits(0x1e3ce508u));
        }
    }
    float next() noexcept {
        const float delta = (target - current) * coefficient;
        if (std::abs(delta) < threshold) current = target;
        else current = current + delta;
        return current;
    }
};

std::int16_t interpolate(const BasicWaveTables::Table& table, std::uint32_t phase) noexcept {
    const auto index = phase >> 23;
    const auto first = table[index];
    const auto next = table[(index + 1) & 0x1ffu];
    const auto delta = short_wrap(static_cast<std::int32_t>(next) - first);
    const auto fraction = static_cast<std::int32_t>((phase >> 7) & 0xffffu);
    const auto part = short_wrap((static_cast<std::int32_t>(delta) * fraction) >> 16);
    return short_wrap(static_cast<std::int32_t>(part) + first);
}

}  // namespace

struct BasicWaves::Impl {
    const BasicWaveTables& tables;
    const BasicWaveTables::Table* main_table;
    const BasicWaveTables::Table* sub_table;
    std::array<std::uint32_t, 3> phase{};
    std::array<std::uint32_t, 3> increment{};
    std::uint32_t seed = 0;
    std::uint32_t pulse_offset = 0;
    std::array<float, 3> controls{};
    Slew wave;
    LowPass symmetry;
    Slew sub;
    std::int16_t wave_mix = 0;
    std::int16_t sub_mix = 0;
    std::array<std::int16_t, 3> gain{32767, 0, 0};
    bool force = false;
    std::uint8_t trigger_suppressed = 1;

    explicit Impl(const BasicWaveTables& source)
        : tables(source), main_table(&source.lookup(BasicWaveform::sine, 0)),
          sub_table(&source.lookup(BasicWaveform::sine, 0)) {}
};

BasicWaves::BasicWaves(const BasicWaveTables& tables) : impl_(std::make_unique<Impl>(tables)) {}
BasicWaves::~BasicWaves() = default;

void BasicWaves::set_pitch(float frequency, float table_key) noexcept {
    impl_->increment[0] = static_cast<std::uint32_t>(static_cast<std::int64_t>(frequency * from_bits(0x472ec33eu)));
    impl_->increment[1] = impl_->increment[0];
    impl_->increment[2] = static_cast<std::uint32_t>(static_cast<std::int64_t>(frequency * from_bits(0x46aec33eu)));
    const auto band = static_cast<std::uint32_t>((static_cast<std::uint64_t>(static_cast<std::int64_t>(table_key)) >> 2) & 31u);
    impl_->main_table = &impl_->tables.lookup(BasicWaveform::saw, band);
    impl_->sub_table = &impl_->tables.lookup(BasicWaveform::sine, band);
}

void BasicWaves::set_phase_seed(float phase) noexcept {
    impl_->seed = static_cast<std::uint32_t>(static_cast<std::int64_t>(phase * from_bits(0x4f800000u)));
}

void BasicWaves::initialize() noexcept {}

void BasicWaves::set_trigger_suppressed(std::uint8_t value) noexcept {
    impl_->trigger_suppressed = value;
}

void BasicWaves::trigger(std::uint8_t flag) noexcept {
    if (flag != 0 && impl_->trigger_suppressed == 0)
        impl_->phase = {impl_->seed, impl_->seed + impl_->pulse_offset, impl_->seed};
}

void BasicWaves::reset() noexcept {
    impl_->phase = {impl_->seed, impl_->seed + impl_->pulse_offset, impl_->seed};
    impl_->wave.reset(impl_->controls[0]);
    impl_->symmetry.reset(impl_->controls[1]);
    impl_->sub.reset(impl_->controls[2]);
    impl_->force = true;
}

void BasicWaves::prepare_controls(const std::array<std::uint16_t, 3>& words,
                                  const std::array<std::uint8_t, 3>& dirty) noexcept {
    constexpr float scale = from_bits(0x37800200u);
    if (dirty[0]) impl_->wave.target = impl_->controls[0] = static_cast<float>(words[0]) * scale;
    if (dirty[1]) {
        impl_->controls[1] = static_cast<float>(words[1]) * scale;
        impl_->symmetry.set_target(impl_->controls[1]);
    }
    if (dirty[2]) impl_->sub.target = impl_->controls[2] = static_cast<float>(words[2]) * scale;
    bool recalculate = false;
    if (!impl_->wave.settled() || impl_->force) {
        const float value = impl_->wave.next();
        impl_->wave_mix = short_wrap(static_cast<std::int32_t>(value * 65534.0f - 32767.0f));
        recalculate = true;
    }
    if (!impl_->symmetry.settled() || impl_->force) {
        const auto value = short_wrap(static_cast<std::int32_t>(impl_->symmetry.next() * 32767.0f));
        const auto offset = ((static_cast<std::int32_t>(value) * 0x3999) >> 15) - 0x4000;
        impl_->pulse_offset = static_cast<std::uint32_t>(offset) * 0x20000u;
        impl_->phase[1] = impl_->phase[0] + impl_->pulse_offset;
    }
    if (!impl_->sub.settled() || impl_->force) {
        impl_->sub_mix = short_wrap(static_cast<std::int32_t>(impl_->sub.next() * 32767.0f));
    } else if (!recalculate) {
        impl_->force = false;
        return;
    }
    const auto remaining = short_wrap((impl_->sub_mix >> 1) - impl_->sub_mix + 0x7fff);
    impl_->gain[0] = remaining;
    impl_->gain[1] = short_wrap((static_cast<std::int32_t>(impl_->wave_mix) * remaining) >> 15);
    impl_->gain[2] = impl_->sub_mix;
    impl_->force = false;
}

void BasicWaves::render(std::span<float, 32> output) noexcept {
    constexpr float scale = from_bits(0x30800000u);
    for (float& sample : output) {
        const auto first = interpolate(*impl_->main_table, impl_->phase[0]);
        const auto second = interpolate(*impl_->main_table, impl_->phase[1]);
        impl_->phase[0] += impl_->increment[0];
        impl_->phase[1] += impl_->increment[1];
        const auto sum = static_cast<std::int64_t>(impl_->gain[0]) * first +
                         static_cast<std::int64_t>(impl_->gain[1]) * second;
        sample = static_cast<float>(sum) * scale;
    }
    for (float& sample : output) {
        const auto value = interpolate(*impl_->sub_table, impl_->phase[2]);
        impl_->phase[2] += impl_->increment[2];
        const auto product = static_cast<std::int32_t>(value) * impl_->gain[2];
        sample = static_cast<float>(static_cast<std::int64_t>(product)) * scale + sample;
    }
}

}  // namespace mfrk
