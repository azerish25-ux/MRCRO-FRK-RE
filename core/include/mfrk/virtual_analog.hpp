#pragma once

#include <memory>
#include <span>

namespace mfrk {

struct VirtualAnalogParameters {
    float note;
    float timbre;
    float morph;
    float harmonics;
};

// Raw recovered engine component. Host controls, resampling and voice routing
// are not part of this interface. Parameters must be finite.
class VirtualAnalog {
public:
    VirtualAnalog();
    ~VirtualAnalog();
    VirtualAnalog(const VirtualAnalog&) = delete;
    VirtualAnalog& operator=(const VirtualAnalog&) = delete;

    void render(const VirtualAnalogParameters& parameters,
                std::span<float, 32> output) noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace mfrk
