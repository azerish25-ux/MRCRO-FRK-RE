#include "mfrk/virtual_analog.hpp"

#include <algorithm>

namespace mfrk {

struct VirtualAnalog::Impl {};

VirtualAnalog::VirtualAnalog() : impl_(std::make_unique<Impl>()) {}
VirtualAnalog::~VirtualAnalog() = default;

// RED phase: original numeric traces must reject this before reconstruction.
void VirtualAnalog::render(const VirtualAnalogParameters&,
                           std::span<float, 32> output) noexcept {
    std::fill(output.begin(), output.end(), 0.0f);
}

}  // namespace mfrk
