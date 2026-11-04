#include "mfrk/pitch.hpp"
#include "pitch_tables.hpp"

#include <algorithm>

namespace mfrk {

float normalized_pitch_frequency(float note) noexcept {
    // Preserve the original float operation order and fractional table truncation.
    const float bounded_note = std::clamp(note - 9.0f, -128.0f, 127.0f);
    const float pitch = bounded_note + 128.0f;
    const int whole = static_cast<int>(pitch);
    const int fraction = static_cast<int>((pitch - static_cast<float>(whole)) * 256.0f);
    const float ratio = detail::kPitchRatioLow[fraction] * detail::kPitchRatioHigh[whole];
    return ratio * 0.00028645832f;
}

}  // namespace mfrk
