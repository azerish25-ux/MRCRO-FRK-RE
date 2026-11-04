#pragma once
#include "mfrk/ui_flex.hpp"
#include "ui_flex_fixture_reader.hpp"
#include <cmath>
#include <stdexcept>
namespace mfrk::ui::flex::test {
inline void require(bool value,const char*message){if(!value)throw std::runtime_error(message);}
inline bool admitted(float x){return std::isfinite(x)&&std::abs(x)<=1000000.f;}
inline void validate(const Box&box,const std::vector<Item>&items,Rect area){
 require(items.size()<=256,"fixture item count");require(box.direction>=0&&box.direction<=3&&box.wrap>=0&&box.wrap<=2&&box.align_content>=0&&box.align_content<=5&&box.align_items>=0&&box.align_items<=3&&box.justify>=0&&box.justify<=5,"fixture enum domain");for(float x:area)require(admitted(x),"fixture finite target rectangle");
 for(const auto&i:items){require(i.align>=0&&i.align<=4&&admitted(i.grow)&&i.grow>=0&&admitted(i.shrink)&&i.shrink>=0&&admitted(i.basis)&&(i.basis>=0||i.basis==-1),"fixture factor/alignment domain");for(int a=0;a<2;++a){for(float x:{i.size[a],i.minimum[a],i.maximum[a]})require(admitted(x)&&(x>=0||x==-1),"fixture dimension domain");for(float x:i.margin[a])require(admitted(x),"fixture signed margin domain");}}
}
inline Item item(Reader&r){r.zeroes(32);Item i;i.order=r.i32();i.grow=r.f32();i.shrink=r.f32();i.basis=r.f32();i.align=r.i32();i.size[0]=r.f32();i.minimum[0]=r.f32();i.maximum[0]=r.f32();i.size[1]=r.f32();i.minimum[1]=r.f32();i.maximum[1]=r.f32();for(auto&m:i.margin)for(auto&v:m)v=r.f32();r.zeroes(4);return i;}
inline Box box(Reader&r){return {r.i32(),r.i32(),r.i32(),r.i32(),r.i32()};}
inline Rect rect(Reader&r){Rect v;for(auto&x:v){x=r.f32();require(std::isfinite(x),"fixture finite rectangle");}return v;}
struct FlatCase {Box box;Rect area;std::vector<Item>items;std::vector<Rect>expected;std::vector<std::array<std::int32_t,4>>edges;};
inline std::vector<FlatCase> read_flat(Reader&r,std::uint32_t count){std::vector<FlatCase>cases;cases.reserve(count);for(std::uint32_t c=0;c<count;++c){FlatCase value;value.box=box(r);value.area=rect(r);auto n=r.u32();require(n<=256,"fixture item count");for(std::uint32_t j=0;j<n;++j){value.items.push_back(item(r));value.expected.push_back(rect(r));std::array<std::int32_t,4>edges;for(auto&x:edges)x=r.i32();value.edges.push_back(edges);}validate(value.box,value.items,value.area);cases.push_back(std::move(value));}r.end();return cases;}
struct TreeCase {Rect area;std::vector<Node>nodes;std::vector<std::vector<Rect>>expected;};
inline void topology(const std::vector<Node>&nodes){require(nodes.size()<=256,"fixture node count");std::vector<int>parents(nodes.size(),0);for(std::size_t j=0;j<nodes.size();++j){const auto&n=nodes[j];require(n.items.size()==n.children.size(),"fixture child count");for(int child:n.children)if(child!=-1){require(child>int(j)&&std::size_t(child)<nodes.size(),"fixture child topology");require(++parents[child]==1,"fixture shared child");}}for(std::size_t j=1;j<nodes.size();++j)require(parents[j]==1,"fixture unreachable node");}
inline std::vector<TreeCase> read_tree(Reader&r,std::uint32_t count){std::vector<TreeCase>cases;cases.reserve(count);for(std::uint32_t c=0;c<count;++c){TreeCase value;value.area=rect(r);auto n=r.u32();require(n<=256,"fixture node count");for(std::uint32_t j=0;j<n;++j){Node node;node.box=box(r);auto size=r.u32();require(size<=256,"fixture item count");std::vector<Rect>expected;for(std::uint32_t i=0;i<size;++i){node.items.push_back(item(r));node.children.push_back(r.i32());expected.push_back(rect(r));}validate(node.box,node.items,value.area);value.nodes.push_back(std::move(node));value.expected.push_back(std::move(expected));}topology(value.nodes);cases.push_back(std::move(value));}r.end();return cases;}
// Check all float endpoints and edge differences BEFORE the primitive's float->int casts.
inline void edge_domain(Rect v){for(float x:v)require(std::isfinite(x),"computed finite rectangle");const float right=v[0]+v[2],bottom=v[1]+v[3];for(float x:{v[0],v[1],right,bottom})require(std::isfinite(x)&&double(x)>=-2147483648.&&double(x)<2147483648.,"computed edge conversion range");const double w=std::trunc(double(right))-std::trunc(double(v[0])),h=std::trunc(double(bottom))-std::trunc(double(v[1]));require(w>=-2147483648.&&w<=2147483647.&&h>=-2147483648.&&h<=2147483647.,"computed edge difference range");}
} // namespace mfrk::ui::flex::test
