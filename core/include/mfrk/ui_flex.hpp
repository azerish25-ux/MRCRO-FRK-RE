#pragma once
#include <array>
#include <cstddef>
#include <vector>
namespace mfrk::ui::flex {
// Caller supplies evaluated native item records; these initializers are adapter
// convenience values, NOT reconstructed native factory/XML defaults.
struct Item {
 int order=0; float grow=0,shrink=1,basis=0; int align=0;
 std::array<float,2> size{50,30},minimum{-1,-1},maximum{-1,-1};
 std::array<std::array<float,2>,2> margin{{{0,0},{0,0}}};
};
struct Box { int direction=0,wrap=0,align_content=0,align_items=0,justify=0; };
using Rect=std::array<float,4>;
struct Node { Box box;std::vector<Item> items;std::vector<int> children; };

// Pure numeric nativeFlex solver. IEEE32/64 nearest, masked FP exceptions,
// no fast-math/reassociation/contraction. At most256 items; input floats finite
// with abs<=1e6. Dimensions/basis: -1 or nonnegative; grow/shrink nonnegative;
// margins signed, with exact-2 auto sentinel. Enum values documented in README.
// Signed target rectangles are admitted. Exception flags/allocator failure,
// Component setters, declarative producers and widget lifecycle are excluded.
std::vector<Rect> layout(const Box& box,const std::vector<Item>& items,Rect area);
// Owned topologically indexed tree: root0; child indices>parent; every nonroot
// exactly one parent; child=-1 means none; max256nodes. Each derived child area
// must satisfy the same finite bound. No cycles/shared nodes/native pointers.
std::vector<std::vector<Rect>> layout_tree(const std::vector<Node>&nodes,Rect area);
} // namespace mfrk::ui::flex
