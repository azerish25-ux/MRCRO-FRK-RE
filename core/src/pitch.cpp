#include "mfrk/pitch.hpp"

namespace mfrk {

// RED phase: remote tests must reject this before the recovered implementation.
float normalized_pitch_frequency(float) noexcept {
    return 0.0f;
}

}  // namespace mfrk
