#include "mfrk/superwave.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cassert>
#include <limits>
namespace mfrk {

    static_assert(sizeof(float)==4 && std::numeric_limits<float>::is_iec559);

    namespace {
        constexpr float bits(std::uint32_t x) noexcept {
            return std::bit_cast<float>(x);
        }
        std::int16_t wrap(std::int32_t x) noexcept {
            return std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(x));
        }
        std::uint32_t increment(float ratio, float hz) noexcept {
            const float pitched = ratio*hz;
            const float scaled = pitched*bits(0x472ec33e);
            return static_cast<std::uint32_t>(static_cast<std::int64_t>(scaled));
        }
    }
    SuperWaveState SuperWaveState::admitted(std::array<std::uint16_t, 3> w, std::array<std::uint8_t, 3> d) noexcept {
        SuperWaveState s {};
        s.words = w;
        s.dirty = d;
        s.current = bits(0x2b8cbccc);
        s.coefficient = bits(0x3d064870);
        s.threshold = bits(0x2b8cbccc);
        s.level = bits(0x3ee6e979);
        return s;
    }

    void SuperWave::set_pitch(float hz, float key) noexcept {
        auto& s = state_;
        const float d = s.detune*bits(0x3cacca1b);
        const float a = d*1.0625f;
        const float b = d+d;
        const float c = b+d;
        const float e = b*1.0625f;
        const float f = e+a;
        s.frequency = hz;
        s.table_key = key;
        s.bands[0] = static_cast<std::uint32_t>(static_cast<std::uint64_t>(static_cast<std::int64_t>(key))>>2)&31u;
        s.tables[0] = {
            s.families[0], s.bands[0]
        };
        s.increment = {
            increment(1, hz), increment(1-d, hz), increment(a+1, hz), increment(1-c, hz), increment(f+1, hz), increment(1-(c+b), hz), increment((f+e)+1, hz)
        };
    }

    void SuperWave::set_phase(float phase) noexcept {
        constexpr std::array<std::uint32_t, 7> offsets {
            0u, 0x3e800000u, 0x3ec00000u, 0x3ef1c715u, 0x3d638da4u, 0x3ef7776cu, 0x3d4ccccdu
        };
        for (std::size_t i = 0;i<7;++i) {
            const float value = i==0?phase:phase+bits(offsets[i]);
            const float integral = static_cast<float>(static_cast<std::int32_t>(value));
            const float fraction = value-integral;
            const float scaled = fraction*bits(0x4f800000u);
            state_.seed[i] = static_cast<std::uint32_t>(static_cast<std::int64_t>(scaled));
            state_.phase[i] = state_.seed[i];
        }
    }

    void SuperWave::initialize() noexcept {
        auto& s = state_;
        s.phase = s.seed;
        s.target = s.current = s.blend;
        s.force = 1;
    }

    void SuperWave::repoint() noexcept {
        auto& s = state_;
        for (std::size_t i = 0;i<7;++i)s.tables[i] = {
            s.families[i], s.bands[i]
        };
    }

    void SuperWave::trigger(std::uint8_t flag) noexcept {
        if (flag&&!state_.trigger_suppressed)state_.phase[0] = state_.seed[0];
    }

    void SuperWave::prepare_controls(bool clear_dirty) noexcept {
        auto& s = state_;
        if (s.dirty[0]) {
            s.wave_quartile = static_cast<std::uint32_t>(s.words[0])*4u>>16;
            s.families[0] = s.wave_quartile;
            s.tables[0] = {
                s.families[0], s.bands[0]
            };
        }
        if (s.dirty[1]) {
            const float x = static_cast<float>(s.words[1]);
            const float square = x*x;
            const float scaled = x*bits(0x27800600);
            s.detune = square*scaled;
            set_pitch(s.frequency, s.table_key);
        }
        if (s.dirty[2]) {
            s.blend = static_cast<float>(s.words[2])*bits(0x37800200);
            if (s.target!=s.blend) {
                s.target = s.blend;
                s.threshold = std::max(std::abs(s.blend)*bits(0x2b8cbccc), bits(0x1e3ce508));
            }
        }
        if (s.current!=s.target||s.force) {
            const float distance = s.target-s.current;
            const float delta = distance*s.coefficient;
            s.current = std::abs(delta)<s.threshold?s.target:s.current+delta;
            const auto v = wrap(static_cast<std::int32_t>(s.current*32767.0f));
            s.blend_fixed = v;
            s.weight[0] = wrap(32767-(v>>1));
            const auto diff = static_cast<std::uint32_t>(32767-static_cast<std::int32_t>(v));
            const auto square = std::bit_cast<std::int32_t>(diff*diff);
            const auto side = wrap(0x4ccc-(square>>16));
            std::fill(s.weight.begin()+1, s.weight.end(), side);
            s.coefficients_initialized = true;
            s.force = 0;
        }
        if (clear_dirty)s.dirty = {
            0, 0, 0
        };
    }

    void render_superwave32(SuperWaveState& s, const mfrk::BasicWaveTables::Table& t, std::span<float, 32> out) noexcept {
        assert(s.coefficients_initialized);
        for (float& value:out) {
            std::int64_t sum = 0;
            for (std::size_t i = 0;i<7;++i) {
                const auto p = s.phase[i];
                const auto index = p>>23;
                const auto first = t[index];
                const auto delta = wrap(static_cast<std::int32_t>(t[(index+1)&511u])-first);
                const auto part = (static_cast<std::int32_t>(delta)*static_cast<std::int32_t>((p>>7)&65535u))>>16;
                const auto sample = wrap(static_cast<std::int32_t>(first)+part);
                sum+= static_cast<std::int32_t>(sample)*static_cast<std::int32_t>(s.weight[i]);
                s.phase[i]+= s.increment[i];
            }
            value = static_cast<float>(sum)*bits(0x30800000);
        }
    }

    void SuperWave::render_raw(std::span<float, 32> out) noexcept {
        const auto& choice = state_.tables[0];
        render_superwave32(state_, tables_.lookup(static_cast<mfrk::BasicWaveform>(choice.family), choice.band), out);
    }

    void SuperWave::render_scaled(std::span<float, 32> out) noexcept {
        render_raw(out);
        for (float& x:out)x = state_.level*x;
    }
}
