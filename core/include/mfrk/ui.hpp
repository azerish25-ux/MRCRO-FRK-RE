#pragma once
#include <array>
#include <cstdint>
#include <span>
namespace mfrk::ui {
// The implementation source must retain strict IEEE32/64 nearest operations,
// no fast-math or FP contraction, gradual underflow and masked FP exceptions.
// Caller-provided values must satisfy these
// admitted finite/representable profiles. These are math primitives, not a renderer.

// Source0x180f64300 interpolation branch: nonnegative integer dimensions, finite
// style/state and intermediates. Caller supplies transition weight; no lifecycle.
std::array<float,4> interpolated_center_bounds(std::int32_t width,
    std::int32_t height,float normal_radius,float hover_radius,
    float normal_center_size,float hover_center_size,float transition) noexcept;

// Source0x1814dd3e0 ordinary packed return lanes. Finite abs(x)<=64.
// Large/special/upper-lane paths and floating-environment side effects excluded.
std::array<float,2> packed_sine_cosine(float x) noexcept;

// Source0x180f64500 gradient-enabled/follow-value-false branch. Nonnegative
// dimensions; resolved finite radius/state; all computed sine arguments abs<=64.
// gradient_angle is multiplied by the source binary32 pi, not radians directly.
std::array<float,4> outline_gradient_points(std::int32_t width,
    std::int32_t height,float right_angle,float left_angle,
    float gradient_angle,float center_radius) noexcept;

// Source0x180f64f80 marker branch after the parameter getter. Nonnegative
// dimensions, finite parameters/intermediates, computed sine arguments abs<=64.
// Producer, reverse/lifecycle flags, stroke/raster/typography are not inferred.
std::array<float,4> center_marker_points(std::int32_t width,std::int32_t height,
    float parameter,bool reverse,float right_angle,float left_angle,
    float center_radius,float start,float marker_width) noexcept;

struct MagneticZone {float position_from,position_to,value_from,value_to;bool skip_at_drag_start;};
struct MagneticResult {float value;bool skip;};
// Source0x180f59930 matching-param/non-wrapping branch. Finite value/delta and
// sorted native zone records; intermediates finite, nonzero interpolated spans.
// Endpoint/plateau/point zones are admitted. Caller owns zone lifetime and flags.
MagneticResult magnetic_delta(std::span<const MagneticZone> zones,
    float value,float delta,bool flag20,bool flag21) noexcept;

// Source0x181071293 edge projection. Coordinates and separately rounded float
// endpoint sums must truncate to signed32; endpoint differences must also fit.
// Returns the pre-Component::setBounds rectangle; no native setter/clamp implied.
std::array<std::int32_t,4> quantize_flex_edges(float x,float y,float width,float height) noexcept;

// Source0x180b89110/b89200 scaled-absolute branch only. Finite nonnegative
// binary32 scale and binary64 logical size; double product+.5 signed32-representable.
std::int32_t scaled_absolute_dimension(float view_scale,double logical_size) noexcept;
} // namespace mfrk::ui
