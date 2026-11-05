#pragma once
#include "mfrk/ui_flex.hpp"
#include "mfrk/ui_runtime_layout.hpp"
#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
namespace mfrk::ui::scene {
using Constants=std::unordered_map<std::string,double>;
using Features=std::unordered_set<std::string>;
using Attributes=std::vector<std::pair<std::string,std::string>>;
using StringConstants=std::map<std::string,std::string>;
using StyleRegistry=std::map<std::string,Attributes>;
using Geometry=runtime::Geometry;
// Caller-owned typed state; convenience/default selected fields are not a full
// native widget factory/lifetime claim. See docs/ui-scene-prefix-admission.md.
struct FlexProperties {std::array<int,11>values{0,0,-1,0,-1,0,-1,0,0,0,0};bool force_scale=true;};
struct ItemInput {
 std::array<int,2> parent_dimensions{};int direction=0;
 bool context_present=true,parent_force_scale=true,child_force_scale=true;
 std::array<bool,2> full_dimension_override{};int dimension_mode=1;
 float view_scale=1,context_d4=1,desktop_scale=1;
 std::array<double,2> relative{},absolute{};
 // Native child724..74c: align-self,order,flex,minW,maxW,minH,maxH,left,right,top,bottom.
 std::array<int,11> properties{0,0,-1,0,-1,0,-1,0,0,0,0};
};
enum class DeclaredParent {none,generic,smart,flex};
struct DeclaredGeometryInput {
 DeclaredParent parent_kind=DeclaredParent::flex;
 FlexProperties initial_properties{{0,0,-1,0,-1,0,-1,-1,-1,-1,-1},true};
 std::array<int,2>parent_dimensions{};
 std::array<int,2>intrinsic{};
 std::optional<std::array<int,4>>previous;
 int initial_mode=1;bool initial_check_bounds=true,context_check_bounds=false;
};
enum class BoundsFailure {none,negative_x,negative_y,right,bottom};
struct DeclaredChild {std::array<double,4>raw{};std::optional<std::array<int,4>>bounds;int mode=1;bool check_bounds=true;FlexProperties properties;BoundsFailure failure=BoundsFailure::none;};
struct GeometryPipelineInput {DeclaredGeometryInput declaration;std::array<int,2>physical_parent{};int direction=2;float view_scale=1,context_d4=1,desktop_scale=1;bool parent_force=true,context_present=true;std::array<bool,2>full_override{};};
struct GeometryPipelineResult {DeclaredChild declaration;std::optional<Geometry>spec;std::optional<flex::Item>item;};
std::optional<std::string_view> attribute_value(std::string_view key,const Attributes&local,const Attributes&style);
const Attributes* resolve_style(const Attributes&local,const StyleRegistry&registry,const StringConstants&strings={},bool strict=true);
std::int32_t read_integer_attribute(std::string_view key,const Attributes&local,const Attributes&style,const Constants&numbers,std::int32_t missing,std::int32_t maximum=2147483647,const Features*features=nullptr);
bool read_boolean_attribute(std::string_view text,const Constants&numbers);
FlexProperties read_flex_properties(const Attributes&local,const Attributes&style,const Constants&numbers,const StringConstants&strings={},bool strict_strings=false,const Features*features=nullptr);
bool select_if(const Attributes&local,const Constants&numbers,const StringConstants&strings={},bool strict_strings=false,const Features*features=nullptr);
flex::Item build_item(const ItemInput&a);
DeclaredChild compile_declared_geometry(const Attributes&local,const Attributes&style,Constants&numbers,const DeclaredGeometryInput&input,const StringConstants&strings={},bool strict=true,const Features*features=nullptr);
DeclaredChild compile_declared_node(const Attributes&local,const StyleRegistry&registry,Constants&numbers,const DeclaredGeometryInput&input,const StringConstants&strings={},bool strict=true,const Features*features=nullptr);
GeometryPipelineResult compile_geometry_pipeline(const Attributes&local,const StyleRegistry&registry,Constants&numbers,const GeometryPipelineInput&input,const StringConstants&strings={},bool strict=true,const Features*features=nullptr);
// Finite source arithmetic; feature() requires an explicit immutable registry.
double evaluate_expression(std::string_view text,const Constants&numbers,const Features*features=nullptr);
// Alias/placeholder phase only, before the separate native localization service.
std::string expand_aliases(std::string_view text,const StringConstants&strings,bool strict);
std::string expand_condition_aliases(std::string_view text,const StringConstants&strings,bool strict);
bool parse_boolean_literal(std::string_view text);
} // namespace mfrk::ui::scene
