#pragma once

namespace mfrk {

// Recovered engine helper. Input must be finite. The output is the internal
// normalized frequency, not a claim about the public host's sample rate.
[[nodiscard]] float normalized_pitch_frequency(float note) noexcept;

}  // namespace mfrk
