#include "mfrk/deferred_vcf49.hpp"
#include "mfrk/filter_resource.hpp"
#include "mfrk/vcf49.hpp"
#include "filter_parameters.hpp"

#include <atomic>
#include <bit>
#include <cassert>

namespace mfrk {
namespace {
namespace parameters = detail::filter_parameters;
using Role = parameters::Role;
using Group = parameters::Group;
Role role(Vcf49Parameter parameter) noexcept {
    const auto id = static_cast<std::uint8_t>(parameter);
    assert(id < 7 && id != 4);
    return static_cast<Role>(id);
}
std::uint32_t description(FilterOversampling factor, FilterKernel kernel) noexcept {
    return static_cast<std::uint32_t>(factor) | (static_cast<std::uint32_t>(kernel) << 16);
}
}  // namespace

struct DeferredVcf49::Impl {
    static_assert(std::atomic<FilterWorkPhase>::is_always_lock_free);
    static_assert(std::atomic<std::uint32_t>::is_always_lock_free);
    struct Request { FilterOversampling factor; FilterKernel kernel; float reset_resonance; };
    parameters::ParameterRecords records = parameters::ParameterRecords::original_dual_constructor_defaults();
    std::unique_ptr<Vcf49> raw;
    std::unique_ptr<FilterResource> resource;
    FilterFade fade{1.0f, 0.02f, 1.0f, 1, 0.0f};
    bool adaptive, initialized = false, ready = true, resume = false;
    bool first = true, force = false, raw_first = true, raw_force = false, rebuild = false;
    bool latency_changed = false;
    float base_rate = 1.0f;
    std::uint32_t latency = 0;
    std::array<PendingFilterParameter,5> pending{};
    std::size_t pending_count = 0;
    std::atomic<FilterWorkPhase> work{FilterWorkPhase::idle};
    std::atomic<std::uint32_t> installed{0}, seed;
    std::atomic<bool> raw_generation_dirty{false};
    Request request{};
    VcfOutput latched_mode = VcfOutput::lowpass;
    bool latched_noise = true;

    Impl(std::uint32_t initial_seed, bool adapt)
        : raw(std::make_unique<Vcf49>(1.0f, initial_seed)), adaptive(adapt), seed(initial_seed) {}

    Request desired() noexcept {
        const auto index = static_cast<std::uint32_t>(records.physical(Role::FactorIndex));
        const auto kernel = static_cast<FilterKernel>(static_cast<std::uint32_t>(records.physical(Role::FilterType)));
        return {select_filter_factor(index, adaptive, base_rate), kernel, 0.0f};
    }

    // Only executed with rendering suspended, or in the explicit direct-build
    // branch. Old generations are destroyed here, never during normal render.
    void build(const Request& requested) {
        auto next_resource = std::make_unique<FilterResource>(requested.factor, requested.kernel, 32);
        auto next_raw = raw->reconfigured(base_rate * static_cast<float>(requested.factor), requested.reset_resonance);
        raw = std::move(next_raw);
        resource = std::move(next_resource);
        installed.store(description(requested.factor, requested.kernel), std::memory_order_relaxed);
        raw_generation_dirty.store(true,std::memory_order_relaxed);
    }

    void complete() noexcept {
        // Native mark-dirty/reset occurs BEFORE queued normalized updates.
        raw->reset(records.normalized(Role::Resonance));
        if (ready) resource->reset();
        raw_first = raw_force = first = force = true;
        records.mark_aggregate_dirty(Group::Raw);
        records.mark_aggregate_dirty(Group::Extra);
        raw_generation_dirty.store(false,std::memory_order_relaxed);
        ready = true;
        for (std::size_t i = 0; i < pending_count; ++i) {
            records.set_normalized(role(pending[i].parameter), std::bit_cast<float>(pending[i].normalized_bits));
        }
        pending_count = 0;
        if (resume) fade.set_target(1.0f);
        else fade.set_both(1.0f);
        resume = false;
        const auto next_latency = resource->latency_frames();
        latency_changed = latency_changed || latency != next_latency;
        latency = next_latency;
    }

    void prepare_raw() noexcept {
        const bool mode_dirty = records.snapshot(Role::FilterMode).dirty;
        const bool noise_dirty = records.snapshot(Role::NoiseOn).dirty;
        if (mode_dirty || raw_force) latched_mode = static_cast<VcfOutput>(static_cast<int>(records.physical(Role::FilterMode)));
        if (noise_dirty || raw_force) latched_noise = records.boolean_value(Role::NoiseOn);
        if (records.snapshot(Role::MF49ModeOn).dirty || raw_force) {
            const bool model49 = records.boolean_value(Role::MF49ModeOn);
            assert(model49); (void)model49;
        }
        VcfControls controls{records.normalized(Role::Cutoff), records.normalized(Role::Resonance),
            latched_mode, latched_noise, {}, raw_first, raw_force};
        for (std::size_t i = 0; i < 4; ++i) controls.dirty[i] = records.snapshot(static_cast<Role>(i)).dirty;
        raw->prepare(controls);
    }
};

DeferredVcf49::DeferredVcf49(std::uint32_t seed, bool adaptive) : impl_(std::make_unique<Impl>(seed,adaptive)) {}
DeferredVcf49::~DeferredVcf49() = default;

void DeferredVcf49::initialize(float rate) {
    auto& s = *impl_;
    assert(s.work.load(std::memory_order_acquire) == FilterWorkPhase::idle);
    s.base_rate = rate;
    const auto previous = s.fade.state();
    s.fade = FilterFade(rate,0.02f,1.0f,1,previous.current);
    s.fade.set_target(previous.target);
    s.ready = false;
    const bool model49 = s.records.boolean_value(Role::MF49ModeOn);
    assert(model49); (void)model49;
    auto desired = s.desired();
    desired.reset_resonance = s.records.normalized(Role::Resonance);
    s.build(desired);
    s.complete();
    s.initialized = true;
}

void DeferredVcf49::set_normalized(Vcf49Parameter parameter, float value) noexcept {
    auto& s = *impl_;
    const auto id = static_cast<std::uint8_t>(parameter);
    const auto parameter_role = role(parameter);
    if (id < 5 && !s.ready) {
        for (std::size_t i = 0; i < s.pending_count; ++i) {
            if (s.pending[i].parameter == parameter) {
                s.pending[i].normalized_bits = std::bit_cast<std::uint32_t>(value); return;
            }
        }
        assert(s.pending_count < s.pending.size());
        s.pending[s.pending_count++] = {parameter,std::bit_cast<std::uint32_t>(value)};
        return;
    }
    s.records.set_normalized(parameter_role,value);
}

void DeferredVcf49::set_physical(Vcf49Parameter parameter, float value) noexcept {
    assert(static_cast<std::uint8_t>(parameter) >= 5 || impl_->work.load(std::memory_order_acquire) == FilterWorkPhase::idle);
    impl_->records.set_physical(role(parameter),value);
}
float DeferredVcf49::normalized(Vcf49Parameter parameter) noexcept { return impl_->records.normalized(role(parameter)); }
float DeferredVcf49::physical(Vcf49Parameter parameter) noexcept { return impl_->records.physical(role(parameter)); }

void DeferredVcf49::prepare(bool direct) {
    auto& s = *impl_;
    assert(s.initialized);
    s.latency_changed = false;
    if (s.work.load(std::memory_order_acquire) == FilterWorkPhase::completed) {
        s.complete(); s.work.store(FilterWorkPhase::idle,std::memory_order_release);
    }
    const bool dirty = s.records.snapshot(Role::FilterType).dirty || s.records.snapshot(Role::FactorIndex).dirty;
    if (!s.ready || !(dirty || s.force || s.rebuild)) return;
    auto desired = s.desired();
    if (!s.rebuild && s.installed.load(std::memory_order_relaxed) == description(desired.factor,desired.kernel)) return;
    s.rebuild = false;
    if (direct) {
        desired.reset_resonance = s.records.normalized(Role::Resonance);
        s.build(desired); s.complete();
    }
    else if (s.first) { s.fade.set_both(0.0f); s.resume = false; }
    else { s.fade.set_target(0.0f); s.resume = true; }
}

bool DeferredVcf49::render(std::span<const float,32> audio, std::span<const float,32> modulation,
                           std::span<float,32> output) noexcept {
    auto& s = *impl_;
    assert(s.initialized);
    if (!s.ready) return false;
    s.prepare_raw();
    s.resource->process(*s.raw,audio,modulation,output);
    s.records.clear_dirty(Group::Raw); s.raw_first = s.raw_force = false;
    s.seed.store(s.raw->noise_state(),std::memory_order_relaxed);
    const auto fade = s.fade.state();
    if (fade.current != 1.0f || fade.target != 1.0f) {
        std::array<float,32> gains;
        s.fade.render(gains);
        for (std::size_t i = 0; i < output.size(); ++i) output[i] = output[i] * gains[i];
    }
    const auto after = s.fade.state();
    if (after.current == 0.0f && after.target == 0.0f) {
        assert(s.work.load(std::memory_order_acquire) == FilterWorkPhase::idle);
        s.ready = false;
        s.request = s.desired();
        s.request.reset_resonance = s.records.normalized(Role::Resonance);
        s.work.store(FilterWorkPhase::requested,std::memory_order_release);
    }
    return true;
}

void DeferredVcf49::end_block() noexcept {
    impl_->first = impl_->force = false;
    impl_->records.clear_dirty(Group::Extra);
}
void DeferredVcf49::reset() noexcept { if (impl_->ready && impl_->resource) impl_->resource->reset(); }
void DeferredVcf49::request_rebuild() noexcept { impl_->rebuild = true; }

bool DeferredVcf49::worker_step() {
    auto& s = *impl_;
    auto expected = FilterWorkPhase::requested;
    if (!s.work.compare_exchange_strong(expected,FilterWorkPhase::building,std::memory_order_acquire)) return false;
    try {
        s.build(s.request);
    } catch (...) {
        // No generation was installed. Preserve the request for an off-callback
        // host retry instead of leaving the audio actor permanently suspended.
        s.work.store(FilterWorkPhase::requested,std::memory_order_release);
        throw;
    }
    s.work.store(FilterWorkPhase::completed,std::memory_order_release);
    return true;
}

DeferredFilterState DeferredVcf49::state() const noexcept {
    const auto& s = *impl_;
    const auto work = s.work.load(std::memory_order_acquire);
    const auto installed = s.installed.load(std::memory_order_relaxed);
    return {s.initialized,s.ready,s.resume,s.first,s.force,s.latency_changed,s.fade.state(),
        installed & 0xffffu,installed >> 16,s.latency,s.seed.load(std::memory_order_relaxed),
        work,s.pending,s.pending_count};
}

FilterParameterSnapshot DeferredVcf49::parameter_snapshot(std::uint32_t id) const noexcept {
    assert(id < 7);
    const auto& s = *impl_;
    // The installed worker generation owns the native Init aggregate mark until
    // completion merges it into actor-owned records. Cache cells stay actor-owned.
    (void)s.work.load(std::memory_order_acquire);
    const auto value = s.records.snapshot(static_cast<Role>(id));
    const bool initialized_raw = id < 5 && s.raw_generation_dirty.load(std::memory_order_relaxed);
    return {value.normalized_bits,value.physical_bits,value.status,value.dirty,value.aggregate_dirty || initialized_raw};
}
}  // namespace mfrk
