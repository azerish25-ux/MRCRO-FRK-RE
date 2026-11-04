#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace mfrk::detail::filter_parameters {

enum class Role : std::uint8_t {
    Cutoff, Resonance, FilterMode, NoiseOn, MF49ModeOn, FilterType, FactorIndex
};
enum class Group : std::uint8_t { Raw, Extra };
enum class CacheStatus : std::uint32_t {
    Synchronized = 0, Normalized = 1, Physical = 2
};

// Status0 means both caches are usable; discrete normalized values need not be canonical.
struct RecordState {
    float normalized{};
    float physical{};
    CacheStatus status{};
    bool dirty{};
};
struct Snapshot {
    std::uint32_t normalized_bits, physical_bits, status;
    bool dirty, aggregate_dirty;
};
struct Metadata {
    std::uint32_t converter_type;
    float item_count, polarity, minimum, maximum;
};

// Fixed native wrapper roles0..6, with their original default empty alias lists.
// Preconditions: valid Role/Group/status; finite normalized values in[0,1];
// finite physical values in the role's source range (item indices for discrete roles).
// Compile without fast-math or floating-point contraction. No allocation or I/O.
class ParameterRecords {
public:
    using States = std::array<RecordState, 7>;
    using Aggregates = std::array<bool, 2>;

    explicit ParameterRecords(States states, Aggregates aggregates) noexcept
        : states_(states), aggregates_(aggregates) {}

    // Captured after FULL original DualVCF ctor2fe390, before Public Init/prepare.
    // Only the represented, observed fields are initialized; no native padding is modeled.
    static ParameterRecords original_dual_constructor_defaults() noexcept {
        return ParameterRecords{States{{
            {0.0f, 0.0f, CacheStatus::Synchronized, false}, // Cutoff
            {0.0f, 0.0f, CacheStatus::Synchronized, false}, // Resonance
            {0.0f, 0.0f, CacheStatus::Synchronized, false}, // FilterMode
            {1.0f, 0.0f, CacheStatus::Normalized, true},    // NoiseOn
            {0.0f, 1.0f, CacheStatus::Physical, true},      // MF49ModeOn
            {0.0f, 0.0f, CacheStatus::Synchronized, false}, // FilterType
            {0.0f, 0.0f, CacheStatus::Synchronized, false}  // FactorIndex
        }}, {true, false}};
    }

    static constexpr Metadata metadata(Role role) noexcept {
        constexpr std::array<Metadata, 7> specs{{
            {1, 2.0f, 1.0f, 0.0f, 100.0f},
            {1, 2.0f, 1.0f, 0.0f, 100.0f},
            {18, 3.0f, 1.0f, 0.0f, 2.0f},
            {0, 2.0f, 1.0f, 0.0f, 1.0f},
            {0, 2.0f, 1.0f, 0.0f, 1.0f},
            {18, 2.0f, 1.0f, 0.0f, 1.0f},
            {18, 4.0f, 1.0f, 0.0f, 3.0f}
        }};
        return specs[index(role)];
    }

    void set_normalized(Role role, float value) noexcept {
        auto& record = states_[index(role)];
        // Held dirty skips lazy comparison, just as the native setter does.
        if (record.dirty || value != normalized(role)) mark_dirty(role);
        // Even an unchanged value is stored verbatim and makes physical cache stale.
        record.normalized = value;
        record.status = CacheStatus::Normalized;
    }

    void set_physical(Role role, float value) noexcept {
        auto& record = states_[index(role)];
        if (record.dirty || value != physical(role)) mark_dirty(role);
        record.physical = value;
        record.status = CacheStatus::Physical;
        // Wrapper extras resolve normalized value before alias fanout, even if empty.
        // Raw IDs0..4 leave status2 until a later normalized read.
        if (group_for(role) == Group::Extra) (void)normalized(role);
    }

    float normalized(Role role) noexcept {
        auto& record = states_[index(role)];
        if (record.status == CacheStatus::Physical) {
            record.normalized = to_normalized(role, record.physical);
            record.status = CacheStatus::Synchronized;
        }
        return record.normalized;
    }

    float physical(Role role) noexcept {
        auto& record = states_[index(role)];
        if (record.status == CacheStatus::Normalized) {
            record.physical = to_physical(role, record.normalized);
            record.status = CacheStatus::Synchronized;
        }
        return record.physical;
    }

    // NoiseOn/MF49ModeOn only. Computes the value; caller owns prepare-time latching.
    bool boolean_value(Role role) noexcept { return physical(role) > 0.5f; }

    void clear_dirty(Group group) noexcept {
        const auto group_index = static_cast<std::size_t>(group);
        if (!aggregates_[group_index]) return;
        const std::size_t begin = group == Group::Raw ? 0 : 5;
        const std::size_t end = group == Group::Raw ? 5 : 7;
        for (std::size_t i = begin; i < end; ++i) states_[i].dirty = false;
        aggregates_[group_index] = false;
    }
    void clear_dirty() noexcept { clear_dirty(Group::Raw); clear_dirty(Group::Extra); }

    // Native wrapper mark-dirty can force the aggregate without individual flags.
    void mark_aggregate_dirty(Group group) noexcept { aggregates_[static_cast<std::size_t>(group)] = true; }

    bool aggregate_dirty(Group group) const noexcept {
        return aggregates_[static_cast<std::size_t>(group)];
    }
    Snapshot snapshot(Role role) const noexcept {
        const auto& record = states_[index(role)];
        return {std::bit_cast<std::uint32_t>(record.normalized),
                std::bit_cast<std::uint32_t>(record.physical),
                static_cast<std::uint32_t>(record.status), record.dirty,
                aggregate_dirty(group_for(role))};
    }

private:
    static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);
    static constexpr std::size_t index(Role role) noexcept {
        return static_cast<std::size_t>(role);
    }
    static constexpr Group group_for(Role role) noexcept {
        return index(role) < 5 ? Group::Raw : Group::Extra;
    }
    void mark_dirty(Role role) noexcept {
        states_[index(role)].dirty = true;
        aggregates_[static_cast<std::size_t>(group_for(role))] = true;
    }
    static float to_physical(Role role, float value) noexcept {
        const auto spec = metadata(role);
        if (spec.converter_type == 0) return value;
        if (spec.converter_type == 1) {
            const float scaled = value * (spec.maximum - spec.minimum);
            return scaled + spec.minimum;
        }
        const float scaled = value * spec.item_count;
        // Nonnegative valid normalized inputs: truncation is the source floor.
        const auto item = static_cast<std::uint32_t>(scaled);
        const auto maximum = static_cast<std::uint32_t>(spec.maximum);
        return static_cast<float>(std::min(item, maximum));
    }
    static float to_normalized(Role role, float value) noexcept {
        const auto spec = metadata(role);
        if (spec.converter_type == 0) return value;
        // For discrete roles this divides by N-1, not the quantizer item count N.
        return (value - spec.minimum) / (spec.maximum - spec.minimum);
    }
    States states_;
    Aggregates aggregates_;
};

} // namespace mfrk::detail::filter_parameters
