// Copyright 2016 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// 2 variable shape oscillators with sync, FM and crossfading.

// Adapted using the installed engine's recovered layout and operation order.

#include "mfrk/virtual_analog.hpp"
#include "mfrk/pitch.hpp"

#include "plaits/dsp/oscillator/variable_saw_oscillator.h"
#include "plaits/dsp/oscillator/variable_shape_oscillator.h"
#include "stmlib/dsp/parameter_interpolator.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>

namespace mfrk {

struct VirtualAnalog::Impl {
    plaits::VariableShapeOscillator primary;
    plaits::VariableShapeOscillator square;
    plaits::VariableSawOscillator saw;
    std::array<float, 32> temporary{};
    float square_gain = 0.0f;
    float saw_gain = 0.0f;

    Impl() {
        primary.Init();
        square.Init();
        saw.Init();
    }
};

namespace {

float detuning(float harmonics) noexcept {
    constexpr float intervals[] = {0.0f, 7.01f, 12.01f, 19.01f, 24.01f};
    const float detune = std::clamp(harmonics * 2.05f - 1.025f, -1.0f, 1.0f);
    const float sign = detune < 0.0f ? -1.0f : 1.0f;
    const float position = sign * detune * 3.9999f;
    const int whole = static_cast<int>(position);
    const float fraction = position - static_cast<float>(whole);
    // Installed code's multiplication order, recovered from 0x18032fef0.
    const float smooth = (3.0f - (fraction + fraction)) * fraction * fraction;
    return (((3.0f - (smooth + smooth)) * smooth *
             (intervals[whole + 1] - intervals[whole])) * smooth + intervals[whole]) * sign;
}

}  // namespace

VirtualAnalog::VirtualAnalog() : impl_(std::make_unique<Impl>()) {}
VirtualAnalog::~VirtualAnalog() = default;

void VirtualAnalog::render(const VirtualAnalogParameters& parameters,
                           std::span<float, 32> output) noexcept {
    const float primary_frequency = normalized_pitch_frequency(parameters.note);
    const float auxiliary_frequency = normalized_pitch_frequency(parameters.note + detuning(parameters.harmonics));
    const float sync_frequency = normalized_pitch_frequency(
        parameters.note + parameters.timbre * parameters.timbre * 48.0f);
    const float shape = std::clamp(parameters.morph * 1.5f, 0.0f, 1.0f);
    const float pulse_width = std::clamp((parameters.morph - 0.66f) * 1.46f + 0.5f, 0.5f, 0.995f);
    impl_->primary.Render(primary_frequency, sync_frequency, pulse_width, shape,
                          output.data(), output.size());

    const float square_width = std::clamp(parameters.timbre * 1.3f - 0.15f, 0.005f, 0.5f);
    const float square_sync = parameters.timbre < 0.5f ? 0.0f :
        (parameters.timbre - 0.5f) * (parameters.timbre - 0.5f) * 192.0f;
    const float square_gain = std::min(parameters.timbre * 8.0f, 1.0f);
    const float saw_position = parameters.morph < 0.5f ? parameters.morph + 0.5f :
        1.0f - (((parameters.morph - 0.5f) + parameters.morph) - 0.5f);
    const float saw_width = std::clamp(saw_position * 1.1f, 0.005f, 1.0f);
    const float saw_shape = std::clamp(10.0f - parameters.morph * 21.0f, 0.0f, 1.0f);
    const float saw_gain = std::clamp((1.0f - parameters.morph) * 8.0f, 0.02f, 1.0f);

    impl_->square.Render(primary_frequency,
                         normalized_pitch_frequency(parameters.note + square_sync),
                         square_width, 1.0f, impl_->temporary.data(), output.size());
    impl_->saw.Render(auxiliary_frequency, saw_width, saw_shape, output.data(), output.size());
    const float normalization = 1.0f / std::max(square_gain, saw_gain);
    stmlib::ParameterInterpolator square_mix(&impl_->square_gain, square_gain * 0.3f * normalization, output.size());
    stmlib::ParameterInterpolator saw_mix(&impl_->saw_gain, saw_gain * 0.5f * normalization, output.size());
    for (std::size_t index = 0; index < output.size(); ++index) {
        const float square_level = square_mix.Next();
        const float saw_level = saw_mix.Next();
        // Temporary branch-only diagnostics for the two remaining exact mismatches.
        if (index == 15 && parameters.note >= 108.0f) {
            std::printf("trace note %.9g: square %08x saw %08x square_gain %08x saw_gain %08x\n",
                        static_cast<double>(parameters.note),
                        std::bit_cast<std::uint32_t>(impl_->temporary[index]),
                        std::bit_cast<std::uint32_t>(output[index]),
                        std::bit_cast<std::uint32_t>(square_level),
                        std::bit_cast<std::uint32_t>(saw_level));
        }
        output[index] = square_level * impl_->temporary[index] + saw_level * output[index];
    }
}

}  // namespace mfrk
