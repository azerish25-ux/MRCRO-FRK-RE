#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
namespace mfrk::ui::scene::structure {
struct XmlNode {
 std::uint32_t type=0;
 std::string name,value;
 std::vector<std::pair<std::string,std::string>> attributes;
 std::vector<XmlNode> children;
};
// Original722ab0 mode0 document0/element1/data2/CDATA3 parser body.
// Ordered duplicates, raw colon names, attribute whitespace, unchecked closing
// names and adjacent top-level nodes retain native behavior. Explicit adapter
// caps: input2MiB, element depth128, nodes32768, attributes8192 per node.
XmlNode parse_native_xml(std::string_view input);
struct StyleNode {
 std::string name,value;
 std::vector<std::pair<std::string,std::string>> attributes;
 std::vector<StyleNode> children;
 bool operator==(const StyleNode&)const=default;
};
using Styles=std::map<std::string,StyleNode>;
// Checks element-only profile; unsupported node types are never discarded.
StyleNode to_style_node(const XmlNode&);
// expanded_title is the separately source-resolved FIRST title attribute.
// New key unique; inherited key already registered. Input attrs are consumed
// by inheritance; input children remain owned. Plain definitions are immutable
// data snapshots, without native pointer-alias lifetime. See admission/error scope.
void define_style(Styles&,StyleNode& input,const std::string& expanded_title);
}
