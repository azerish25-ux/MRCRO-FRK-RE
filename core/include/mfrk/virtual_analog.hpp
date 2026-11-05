#pragma once

#include <cstddef>
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
// are not part of this interface. Parameters must be finite; output length
// must be in [1, 32]. The recovered instrument wrapper requests 16 frames.
class VirtualAnalog {
public:
    VirtualAnalog();
    // Fixed borrowed slot scratch; storage must outlive this engine.
    explicit VirtualAnalog(std::span<std::byte,128> scratch);
    ~VirtualAnalog();
    VirtualAnalog(const VirtualAnalog&) = delete;
    VirtualAnalog& operator=(const VirtualAnalog&) = delete;

    // Native inner Init; preserves the outer wrapper interpolation history.
    void initialize() noexcept;

    void render(const VirtualAnalogParameters& parameters,
                std::span<float> output) noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace mfrk
