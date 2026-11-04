#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>
namespace mfrk::ui::runtime {
static_assert(sizeof(int)==4 && sizeof(float)==4 && sizeof(double)==8);
static_assert(std::numeric_limits<float>::is_iec559 && std::numeric_limits<double>::is_iec559);
// Caller-initialized typed numerical state, not native factory/XML defaults.
struct Geometry { std::array<double,4> relative{},absolute{}; };
struct RuntimeLayoutItem {
 std::array<int,4>bounds{},reference_bounds{};float cached_length=0,view_scale=1;
 // visible is the separate Component byte. The four native item bytes are
 // fixed(+8), keep_ratio(+9), opaque_flag_a(+a), fixed_current(+b).
 // C0 ignores+a; parent fixed-cache refresh uses current extent when+a OR+b.
 bool visible=true,fixed=false,keep_ratio=false,opaque_flag_a=false,fixed_current=false;
 int minimum=0,maximum=2147483647;
};
struct RuntimeLayout {
 bool horizontal=false;std::array<int,2>parent_dimensions{};std::array<float,2>background_dimensions{};
 float view_scale=1;std::array<int,2>owner_cc_d0{-1,-1};bool fill_cross=true;
};
struct RuntimeLayoutState {RuntimeLayout layout;std::vector<RuntimeLayoutItem>items;std::vector<Geometry>geometry;bool enabled=true;};
// IEEE32/64 nearest, masked exceptions, strict FP/no contraction required.
// Admission, state-transition ordering and partial-mutation exception behavior
// are documented in README. No native callback, pointer or scene interpreter.
Geometry compile_geometry(std::array<int,2>parent,std::array<int,4>child);
std::optional<std::array<int,4>> resize_child(std::array<int,2>parent,const Geometry&geometry,int mode,bool context_present,float view_scale,float context_d4);
Geometry update_runtime_geometry(std::array<float,2>background,float view_scale,std::array<int,4>bounds);
float native_round(float value);
float native_trunc(float value);
std::vector<Geometry> update_layout(const RuntimeLayout&a,std::vector<RuntimeLayoutItem>&items);
// One ordinary Layout observer; all16 item flag combinations; source fixed
// refresh before guards, child-size cache refresh, then guarded owner reentry.
// This entry always fills the cross axis; fill_cross selects only direct C0.
void parent_resized(RuntimeLayoutState&state);
}
