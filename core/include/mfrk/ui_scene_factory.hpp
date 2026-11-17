#pragma once
#include "mfrk/ui_scene_scope.hpp"
#include "mfrk/ui_component_visibility.hpp"
namespace mfrk::ui::scene::factory {
using Node=scope::Node;
struct Construction {GuiVisibilityComponent* component;bool build_children;};
// Scene-owned widgets survive construction and their post-build callback. These
// services must execute the actual class constructors, parent attachment,
// property/controller registration and include loader; no placeholder fallback.
struct Services {
 virtual ~Services()=default;
 // Source9b1ed0/96a550 generic component attribute/style lookup. Missing
 // component attribute defaults to smartcomponent; explicit empty remains empty.
 virtual std::string generic_component_kind(const Node&,scope::Context&)=0;
 virtual std::string expand_component_string(std::string_view,scope::Context&)=0;
 virtual Construction construct(const Node&,std::string_view kind,std::string_view title,scope::Context&)=0;
 // Source9b0bf0, including visibility/add-child, geometry, parent child-added
 // callback and actual scene-name registration, in their original order.
 virtual void attach(GuiVisibilityComponent& parent,GuiVisibilityComponent& child,const Node&,scope::Context&)=0;
 virtual void finish(GuiVisibilityComponent&)=0; // Source widget v1a0.
 virtual GuiVisibilityComponent* previous_child(GuiVisibilityComponent& parent)=0; // b6f7d0, actual title lookup.
 virtual void include(const Node&,GuiVisibilityComponent& parent,std::uint32_t depth,scope::Context&)=0;
};
// Original signed-char djb2 dispatch in9b29e0; native compares the hash alone.
std::uint64_t component_kind_hash(std::string_view)noexcept;
// Source9b1b50/9b1c40/9b2120 construction orchestration. The scope walker emits
// complete terminal subtrees while the real factory owns their later traversal.
class Builder {
 Services& services_;
 void component(const Node&,GuiVisibilityComponent&,std::uint32_t,scope::Context&);
 void terminal(const Node&,GuiVisibilityComponent&,std::uint32_t,scope::Context&);
 void refresh_parent(GuiVisibilityComponent&,scope::Context&);
public:
 explicit Builder(Services& services):services_(services){}
 void children(const Node&,GuiVisibilityComponent& parent,scope::Context&,std::uint32_t depth=0);
};
}
