#pragma once

#include "mfrk/filter_resampling.hpp"

#include <array>
#include <cstddef>
#include <memory>

namespace mfrk {

// Native local IDs; model flag4 is permanently true in this Dual child.
enum class Vcf49Parameter : std::uint8_t {
    cutoff = 0, resonance = 1, output_mode = 2, noise = 3, kernel = 5, factor_index = 6
};
enum class FilterWorkPhase : std::uint8_t { idle, requested, building, completed };
struct PendingFilterParameter { Vcf49Parameter parameter; std::uint32_t normalized_bits; };
struct FilterParameterSnapshot {
    std::uint32_t normalized_bits, physical_bits, status;
    bool dirty, aggregate_dirty;
};
struct DeferredFilterState {
    bool initialized, ready, resume_fade, first, force, latency_changed;
    FilterFadeState fade;
    std::uint32_t factor, kernel, latency, noise_seed;
    FilterWorkPhase work;
    std::array<PendingFilterParameter, 5> pending;
    std::size_t pending_count;
};

// Control methods/prepare/render/end/snapshot are one actor. Worker step runs
// off-callback; setters on the normalized path remain valid while it builds.
// Physical-origin raw configuration is only admitted with an idle worker.
// Destruction requires the host to join any worker_step execution first.
// Arbitrary native captured state-command replay/OS timing is not implemented.
class DeferredVcf49 {
public:
    explicit DeferredVcf49(std::uint32_t seed, bool adaptive);
    ~DeferredVcf49();
    DeferredVcf49(const DeferredVcf49&) = delete;
    DeferredVcf49& operator=(const DeferredVcf49&) = delete;

    void initialize(float base_rate); // Off callback; fixed native32-frame blocks.
    void set_normalized(Vcf49Parameter parameter, float value) noexcept;
    void set_physical(Vcf49Parameter parameter, float value) noexcept;
    [[nodiscard]] float normalized(Vcf49Parameter parameter) noexcept;
    [[nodiscard]] float physical(Vcf49Parameter parameter) noexcept;
    void prepare(bool direct_build_context = false);
    // Returns false without touching output or DSP when a resource is unavailable.
    bool render(std::span<const float,32> audio, std::span<const float,32> modulation,
                std::span<float,32> output) noexcept;
    void end_block() noexcept;
    void reset() noexcept; // Original wrapper reset: resampler state only, ready-guarded.
    void request_rebuild() noexcept;
    bool worker_step(); // On allocation failure throws with the request still queued.
    [[nodiscard]] DeferredFilterState state() const noexcept;
    // Actor-thread diagnostic cache view, including fixed model record4.
    [[nodiscard]] FilterParameterSnapshot parameter_snapshot(std::uint32_t native_id) const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace mfrk
