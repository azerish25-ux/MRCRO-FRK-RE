#include "filter_parameters.hpp"
#include "filter_parameter_vectors.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <iostream>
#include <string_view>

namespace p = mfrk::detail::filter_parameters;
namespace g = original_parameter_reference;
namespace {
float number(std::uint32_t bits) { return std::bit_cast<float>(bits); }
std::uint32_t bits(float number) { return std::bit_cast<std::uint32_t>(number); }
p::Role role(std::uint32_t id) { return static_cast<p::Role>(id); }
p::Group group(std::uint32_t id) { return id < 5 ? p::Group::Raw : p::Group::Extra; }
p::RecordState record(const g::State& source) {
    return {number(source.normalized_bits), number(source.physical_bits),
            static_cast<p::CacheStatus>(source.status), source.dirty};
}
p::ParameterRecords restore(std::uint32_t id, const g::State& source) {
    // Other records are explicit neutral unit fixtures, not native defaults.
    p::ParameterRecords::States states{};
    p::ParameterRecords::Aggregates dirty{};
    states[id] = record(source);
    dirty[static_cast<std::size_t>(group(id))] = source.aggregate_dirty;
    return p::ParameterRecords{states, dirty};
}
bool same(const p::Snapshot& actual, const g::State& expected) {
    return actual.normalized_bits == expected.normalized_bits &&
           actual.physical_bits == expected.physical_bits &&
           actual.status == expected.status && actual.dirty == expected.dirty &&
           actual.aggregate_dirty == expected.aggregate_dirty;
}
std::size_t checked = 0;
std::size_t failures = 0;
void finish(bool ok, std::string_view category, std::size_t index, bool trace = true) {
    checked += trace;
    if (!ok) {
        if (failures < 10) std::cerr << category << '[' << index << "] mismatch\n";
        ++failures;
    }
}
}
int main() {
    for (std::size_t i = 0; i < g::normalized_cases.size(); ++i) {
        const auto& test = g::normalized_cases[i];
        auto records = restore(test.id, test.before);
        const auto id = role(test.id);
        records.set_normalized(id, number(test.input));
        bool ok = same(records.snapshot(id), test.after_set);
        ok &= bits(records.physical(id)) == test.after_physical.physical_bits;
        ok &= same(records.snapshot(id), test.after_physical);
        ok &= bits(records.normalized(id)) == test.normalized_return;
        ok &= same(records.snapshot(id), test.after_normalized);
        if (test.has_effective) {
            const auto actual = test.id == 2
                ? static_cast<std::uint32_t>(records.physical(id))
                : static_cast<std::uint32_t>(records.boolean_value(id));
            ok &= actual == test.effective;
        }
        finish(ok, "normalized", i);
    }
    std::size_t dirty_index = 0;
    for (const auto& test : g::dirty_cases) {
        auto records = restore(test.id, test.before);
        for (const auto& step : test.steps) {
            if (test.clear_each) records.clear_dirty();
            records.set_normalized(role(test.id), number(step.input));
            finish(same(records.snapshot(role(test.id)), step.after_set), "dirty", dirty_index++);
        }
    }
    for (std::size_t i = 0; i < g::physical_cases.size(); ++i) {
        const auto& test = g::physical_cases[i];
        auto records = restore(test.id, test.before);
        records.clear_dirty();
        const auto id = role(test.id);
        records.set_physical(id, number(test.input));
        bool ok = same(records.snapshot(id), test.after_set);
        ok &= bits(records.physical(id)) == test.after_physical.physical_bits;
        ok &= same(records.snapshot(id), test.after_physical);
        ok &= bits(records.normalized(id)) == test.normalized_return;
        ok &= same(records.snapshot(id), test.after_normalized);
        finish(ok, "physical-origin", i);
    }
    for (std::size_t i = 0; i < g::origin_cases.size(); ++i) {
        const auto& test = g::origin_cases[i];
        auto records = restore(test.id, test.before);
        records.clear_dirty();
        const auto id = role(test.id);
        records.set_normalized(id, number(test.input));
        bool ok = same(records.snapshot(id), test.after_set);
        ok &= bits(records.physical(id)) == test.after_physical.physical_bits;
        ok &= same(records.snapshot(id), test.after_physical);
        finish(ok, "origin-normalized-dirty", i);
    }
    for (std::size_t i = 0; i < g::aggregate_cases.size(); ++i) {
        const auto& test = g::aggregate_cases[i];
        p::ParameterRecords::States states{};
        states[test.changed_id] = record(test.before_changed);
        states[test.unchanged_id] = record(test.before_unchanged);
        p::ParameterRecords records{states, {false, false}};
        records.set_normalized(role(test.changed_id), number(test.changed_input));
        records.set_normalized(role(test.unchanged_id), number(test.unchanged_input));
        finish(same(records.snapshot(role(test.changed_id)), test.after_changed) &&
               same(records.snapshot(role(test.unchanged_id)), test.after_unchanged), "aggregate", i);
    }
    auto original = p::ParameterRecords::original_dual_constructor_defaults();
    for (std::size_t i = 0; i < g::constructor_states.size(); ++i)
        finish(same(original.snapshot(role(static_cast<std::uint32_t>(i))),
                    g::constructor_states[i]), "constructor", i, false);
    for (const auto& row : g::metadata_rows) {
        const auto spec = p::ParameterRecords::metadata(role(row.id));
        finish(spec.converter_type == row.converter && bits(spec.item_count) == row.count &&
               bits(spec.polarity) == row.polarity && bits(spec.minimum) == row.minimum &&
               bits(spec.maximum) == row.maximum, "metadata", row.id, false);
    }
    if (checked != 310) {
        std::cerr << "fixture count mismatch: " << checked << '\n';
        ++failures;
    }
    std::cout << checked << " original trace cases, " << g::constructor_states.size()
              << " constructor records, " << g::metadata_rows.size()
              << " metadata rows; " << failures << " mismatches\n";
    return failures ? 1 : 0;
}

