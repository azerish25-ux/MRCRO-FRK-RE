#pragma once
#include "mfrk/ui_scene_prefix.hpp"
#include "mfrk/ui_scene_structure.hpp"
#include <functional>
namespace mfrk::ui::scene::scope {
using Node=mfrk::ui::scene::structure::XmlNode;
struct Context {
 mfrk::ui::scene::Constants numbers;
 mfrk::ui::scene::StringConstants strings;
 const mfrk::ui::scene::Features* features=nullptr;
};
struct Bindings {
 std::vector<std::pair<std::string,std::string>> strings;
 std::vector<std::pair<std::string,double>> numbers;
};
// Syntax2 string/double with profile. Numeric results may be nonfinite; the
// source CVTT string alias is guarded. No NaN payload/FPU status contract.
// String context:4096 entries/4096-byte values; raw inputs/keys1024, output4096.
// These are adapter admissions, not native hard limits. Item_list/color excluded.
Bindings collect_with(const Node&,const Context&);
// Walk a parent's structural siblings; ordinary nodes are emitted as complete
// terminal subtrees. The receiver owns subsequent widget/factory traversal.
// With and if/elseif/else plus attribute metadata are admitted here. Other
// known structural constructs fail explicitly. Inactive clauses are not evaluated.
using Emit=std::function<void(const Node&,Context&)>;
void walk_nodes(const Node& parent,Context&,const Emit&);
}
