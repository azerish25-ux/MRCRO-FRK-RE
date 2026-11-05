#pragma once
#include "mfrk/ui_scene_prefix.hpp"
#include "mfrk/ui_scene_structure.hpp"
#include <functional>
namespace mfrk::ui::scene::scope {
using Node=mfrk::ui::scene::structure::XmlNode;
enum class TemplateKind { number, string, integer };
struct TemplateParameter { std::string name;TemplateKind kind;std::string default_value; };
struct TemplateDefinition { std::vector<TemplateParameter> parameters;Node body; };
using Templates=std::map<std::string,TemplateDefinition>;
// Bounded syntax-2 template declarations with string/double attributes.
// Registration stores an owning snapshot; style inheritance is separate.
void register_template(Templates&,const Node&);
struct Context {
 mfrk::ui::scene::Constants numbers;
 mfrk::ui::scene::StringConstants strings;
 const mfrk::ui::scene::Features* features=nullptr;
 // Supplies the original parameter's ordered item names to item_list(name).
 // The caller owns parameter lookup and the lifetime of its underlying data.
 std::function<std::vector<std::string>(std::string_view)> item_list;
 // Caller supplies source-registered templates; parsing/registration and
 // predefined-style argument inheritance are separate stages.
 const Templates* templates=nullptr;
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
// With, if/elseif/else, bounded numeric ranges, nonnumeric parameter item lists
// and supplied string/number/integer templates are admitted here.
// Bit 0 of flags enables numeric aliases for range and index values. Unsupported
// loop forms and other structural constructs fail explicitly.
using Emit=std::function<void(const Node&,Context&)>;
void walk_nodes(const Node& parent,Context&,const Emit&,unsigned flags=0);
}
