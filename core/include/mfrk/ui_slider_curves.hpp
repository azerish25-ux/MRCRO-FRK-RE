#pragma once
#include "mfrk/ui_slider_value.hpp"
#include <array>
#include <memory>
#include <string_view>
#include <vector>
namespace mfrk::ui {
enum class SliderCurveKind:std::uint8_t {linear,reverse,inverse,piecewise,harmonic,table,envelope_time,mixer_time};
// Fields correspond to source curve offsets+8/+c/+10/+14. Table includes the
// final guard entry: the forward scale is size-2, not size-1.
struct SliderCurveDefinition {SliderCurveKind kind;std::array<float,4> fields{};std::vector<float> table;};
// Owned concrete source mappings. Finite inputs/intermediates and nonzero
// divisors; table forward input0..1, sorted strict table3..4096. Other mapping
// classes are unsupported, never replaced with a linear curve. Envelope time
// uses the fixed 21 source knots and clamps finite inputs at both endpoints.
// Mixer time uses its separate 10 knots and source reciprocal-multiply inverse.
std::unique_ptr<SliderCurve> make_slider_curve(const SliderCurveDefinition&);
std::unique_ptr<SliderCurve> make_slider_curve(std::string_view native_rtti,
 std::array<float,4> fields={},std::vector<float> table={});
}
