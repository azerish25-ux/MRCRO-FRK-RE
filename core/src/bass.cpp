#include "mfrk/bass.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
namespace mfrk {
    static_assert(sizeof(float)==4&&std::numeric_limits<float>::is_iec559);
    namespace {
        constexpr float bits(std::uint32_t b) noexcept {
            return std::bit_cast<float>(b);
        }
        float smooth(float current, float target) noexcept {
            const float delta = target-current;
            float v = std::min(std::abs(delta)+std::abs(delta), 1.0f);
            v = v*v;
            v = std::max(v*0.25f, bits(0x3dcccccd));
            return v*delta+current;
        }
        float fractional(float p) noexcept {
            return p-static_cast<float>(static_cast<std::int32_t>(p));
        }
    }
    BassState BassState::admitted(std::array<std::uint16_t, 3>w, std::array<std::uint8_t, 3>d) noexcept {
        BassState s {};
        s.words = w;
        s.dirty = d;
        s.period = bits(0x372ec33e);
        s.level = bits(0x3faa595c);
        return s;
    }

    void Bass::trigger(std::uint8_t flag) noexcept {
        if (flag&&!s_.trigger_suppressed) {
            s_.phase_a = s_.phase_seed;
            s_.phase_b = fractional(s_.phase_seed+0.25f);
        }
    }

    void Bass::set_phase(float phase) noexcept {
        s_.phase_seed = phase;
    }

    void Bass::initialize() noexcept {
        s_.target = {0, 0, 0};
    }

    void Bass::prepare_controls(bool clear_dirty) noexcept {
        for (unsigned j = 0; j < 3; ++j) {
            if (s_.dirty[j]) {
                s_.target[j] = static_cast<float>(s_.words[j]) * bits(0x37800200);
            }
        }
        if (clear_dirty)s_.dirty = {
            0, 0, 0
        };
    }

    void Bass::render_raw(std::span<float, 32> output) noexcept {
        s_.copied_frequency = s_.frequency;
        for (unsigned j = 0; j < 3; ++j) {
            s_.smoothed[j] = smooth(s_.smoothed[j], s_.target[j]);
        }
        const float x = s_.smoothed[0], y = s_.smoothed[1], z = s_.smoothed[2];
        const float inc = s_.period*s_.copied_frequency;
        const float a = (1.0f-x)+x*bits(0x3f333333);
        const float b = (1.0f-y)+y*bits(0x3f333333);
        const float c = (1.0f-z)+z*bits(0x3f4ccccd);
        const float gain = (b*a)*c;
        const float scale = (y+y)+0.5f;
        float p = s_.phase_a, q = s_.phase_b;
        auto random = s_.random;
        for (unsigned j = 0;j<32;++j) {
            const float noise = (static_cast<float>(random&255u)*bits(0x3c000000)-1.0f)*z;
            const float jitter = (noise*inc)*bits(0x3ecccccd);
            p = fractional(p+(jitter+inc));
            const float slope1 = p>0.5f?-4.0f-y*24.0f:y*24.0f+4.0f;
            float d = std::abs(p-0.5f);
            const float tri1 = (1.0f-(d+d))*d;
            q = fractional(q+(inc-jitter));
            const float slope2 = q>0.5f?-8.0f-x*120.0f:x*120.0f+8.0f;
            d = std::abs(q-0.5f);
            const float tri2 = (1.0f-(d+d))*d;
            const float shaped = std::min(std::max(tri2*slope2, -2.0f), 2.0f);
            const float square = (shaped*0.25f)*shaped;
            float soft = shaped>0?std::abs(shaped)-square:square-std::abs(shaped);
            soft = soft*a;
            float wave = (noise*bits(0x3d4ccccd)+soft)*(slope1*tri1);
            wave = wave+soft*scale;
            wave = wave+1.0f;
            wave = std::abs(wave);
            wave = std::abs(2.0f-wave);
            wave = std::abs(wave+jitter);
            wave = std::abs(2.0f-wave)-1.0f;
            output[j] = wave*gain;
                if ((j&15u)==0) {
                    random^=random<<13;
                    random^=random>>17;
                    random^=random<<5;
                }
            }
            s_.phase_a = p;
            s_.phase_b = q;
            s_.random = random;
        }
        void Bass::render_scaled(std::span<float, 32> output) noexcept {
            render_raw(output);
            for (float& value:output)value = s_.level*value;
        }
    }
