#include "mfrk/ui_flex.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <stdexcept>
namespace mfrk::ui::flex {
namespace {
struct State { std::size_t index; std::array<double,2> size{},preferred{}; std::array<std::array<double,2>,2> margin{}; bool locked=false; };
struct Line { std::vector<std::size_t> states; double cross=0,position=0,total=0; };
}
std::vector<Rect> layout(const Box& box,const std::vector<Item>& items,Rect area) {
 const std::size_t n=items.size();
 if(n>256||box.direction<0||box.direction>3||box.wrap<0||box.wrap>2||box.align_content<0||box.align_content>5||box.align_items<0||box.align_items>3||box.justify<0||box.justify>5)throw std::invalid_argument("unsupported Flex configuration");
 auto finite=[](float x){return std::isfinite(x)&&std::abs(x)<=1000000.f;};
 for(float x:area)if(!finite(x))throw std::invalid_argument("Flex area outside finite domain");
 for(const auto&i:items){
  if(i.align<0||i.align>4||!finite(i.grow)||i.grow<0||!finite(i.shrink)||i.shrink<0||!finite(i.basis)||(i.basis<0&&i.basis!=-1))throw std::invalid_argument("Flex item factor domain");
  for(int a=0;a<2;++a){for(float x:{i.size[a],i.minimum[a],i.maximum[a]})if(!finite(x)||(x<0&&x!=-1))throw std::invalid_argument("Flex dimension domain");
   for(float x:i.margin[a])if(!finite(x))throw std::invalid_argument("Flex margin domain");}
 }
 std::vector<Rect> result(n);if(n==0)return result;
 const int main=box.direction<2?0:1,cross=1-main;
 const double length=area[2+main],cross_length=area[2+cross];
 auto preferred=[&](const Item&i,int axis,bool use_basis) {
  float p=use_basis&&i.basis>0?i.basis:(i.size[axis]!=-1?i.size[axis]:i.minimum[axis]);
  if(i.minimum[axis]!=-1&&p<i.minimum[axis])return double(i.minimum[axis]);
  if(i.maximum[axis]!=-1&&i.maximum[axis]<p)return double(i.maximum[axis]);
  return double(p);
 };
 std::vector<State> states;states.reserve(n);
 for(std::size_t i=0;i<n;++i){State s;s.index=i;s.preferred={preferred(items[i],0,main==0),preferred(items[i],1,main==1)};states.push_back(s);}
 std::stable_sort(states.begin(),states.end(),[&](const State&a,const State&b){return items[a.index].order<items[b.index].order;});
 auto reset=[&](State&s){s.size=s.preferred;const auto&i=items[s.index];for(int a=0;a<2;++a)for(int e=0;e<2;++e)s.margin[a][e]=i.margin[a][e]==-2?0:double(i.margin[a][e]);};
 auto extent=[](const State&s,int a){return (s.margin[a][0]+s.size[a])+s.margin[a][1];};
 std::vector<Line> lines(1);double remaining=length;bool first=true;
 for(std::size_t j=0;j<n;++j){auto&s=states[j];reset(s);double occupied=extent(s,main);
  if(box.wrap!=0&&remaining<occupied){if(!first)lines.emplace_back();remaining=length;}
  remaining-=occupied;lines.back().states.push_back(j);first=false;
 }
 // 106f390 + 10704d0 + 1070ac0: reset unlocked states, distribute additive factors,
 // freeze clamped items and retry at most itemCount iterations.
 for(auto&line:lines){
  for(auto j:line.states){states[j].locked=false;states[j].size=states[j].preferred;}
  for(std::size_t attempt=0;attempt<n;++attempt){
   for(auto j:line.states)if(!states[j].locked)states[j].size=states[j].preferred;
   double available=length,unlocked_length=0,total_grow=0,total_shrink=0;
   for(auto j:line.states){auto&s=states[j];const auto&i=items[s.index];if(s.locked)available-=extent(s,main);else{unlocked_length+=extent(s,main);total_grow+=double(i.grow);total_shrink+=double(i.shrink);}}
   double difference=available-unlocked_length,unit=0;
   if(difference>0){if(total_grow!=0)unit=difference/total_grow;}else if(total_shrink!=0)unit=difference/total_shrink;
   bool finished=true;
   for(auto j:line.states){auto&s=states[j];if(s.locked)continue;const auto&i=items[s.index];double p=preferred(i,main,true),proposed=p+double(difference>0?i.grow:i.shrink)*unit;
    if(i.maximum[main]!=-1&&proposed>double(i.maximum[main])){s.size[main]=i.maximum[main];s.locked=true;finished=false;}
    else if(p!=-1&&proposed<double(i.minimum[main])){s.size[main]=i.minimum[main];s.locked=true;finished=false;}
    else s.size[main]=proposed;
    line.total+=extent(s,main);
   }
   if(finished)break;
  }
 }
 // Auto margins on main axis use original accumulated line total.
 for(auto&line:lines){double count=0;for(auto j:line.states)for(int e=0;e<2;++e)if(items[states[j].index].margin[main][e]==-2)count+=1;
  double each=(length-line.total)/count;
  if(each>0)for(auto j:line.states)for(int e=0;e<2;++e)if(items[states[j].index].margin[main][e]==-2)states[j].margin[main][e]=each;
 }
 // Cross sizes, then explicit max clamp.
 for(auto&line:lines){if(box.wrap==0)line.cross=cross_length;else for(auto j:line.states)line.cross=std::max(line.cross,extent(states[j],cross));}
 for(auto&line:lines)for(auto j:line.states){auto&s=states[j];for(int a=0;a<2;++a)if(items[s.index].maximum[a]!=-1&&s.size[a]>items[s.index].maximum[a])s.size[a]=items[s.index].maximum[a];}
 double total_cross=0;for(auto&line:lines)total_cross+=line.cross;
 const std::size_t rows=lines.size();
 if(box.align_content==1){for(std::size_t r=0;r<rows;++r)lines[r].position=r==0?0:lines[r-1].cross+lines[r-1].position;}
 else if(box.align_content==2){for(std::size_t r=0;r<rows;++r){double suffix=0;for(std::size_t j=r;j<rows;++j)suffix+=lines[j].cross;lines[r].position=cross_length-suffix;}}
 else if(box.align_content==0){double extra=std::max(0.,(cross_length-total_cross)/double(rows));for(std::size_t r=0;r<rows;++r){lines[r].cross=extra+lines[r].cross;lines[r].position=r==0?0:lines[r-1].cross+lines[r-1].position;}}
 else if(box.align_content==3){for(std::size_t r=0;r<rows;++r)lines[r].position=r==0?(cross_length-total_cross)*.5:lines[r-1].cross+lines[r-1].position;}
 else if(box.align_content==4){double extra=rows<=1?0:std::max(0.,(cross_length-total_cross)/double(rows-1));for(std::size_t r=1;r<rows;++r)lines[r].position=((extra+lines[r-1].position)+lines[r-1].cross)+lines[r].position;}
 else if(box.align_content==5){double extra=rows<=1?0:std::max(0.,(cross_length-total_cross)/double(rows*2));lines[0].position=extra;for(std::size_t r=1;r<rows;++r)lines[r].position=(((extra+extra)+lines[r-1].position)+lines[r].position)+lines[r-1].cross;}
 // Cross-axis auto margins and alignSelf/alignItems. AlignSelf 4 means stretch.
 for(auto&line:lines)for(auto j:line.states){auto&s=states[j];const auto&i=items[s.index];float start=i.margin[cross][0],end=i.margin[cross][1];
  if(start==-2){s.margin[cross][0]=end==-2?(line.cross-s.size[cross])*.5:(line.cross-s.size[cross])-double(end);}
  if(start==-2||end==-2)continue;
  int align=i.align==0?box.align_items:(i.align==4?0:i.align);
  if(align==1||align==0)s.margin[cross][0]=start;
  else if(align==2)s.margin[cross][0]=(line.cross-s.size[cross])-double(end);
  else if(align==3)s.margin[cross][0]=(((line.cross-s.size[cross])-double(start))-double(end))*.5+double(start);
  if(align==0){double value=i.size[cross]==-1?(line.cross-double(start))-double(end):preferred(i,cross,false);
   if(i.maximum[cross]!=-1)value=std::min(value,double(i.maximum[cross]));
   if(i.minimum[cross]!=-1)value=std::max(value,double(i.minimum[cross]));s.size[cross]=value;
  }
 }
 // 10708d0 recomputes totals; 106ff40 places main-axis coordinates in binary32.
 double extra_begin=0,extra_end=0;
 for(auto&line:lines){line.total=0;for(auto j:line.states)line.total+=extent(states[j],main);
  double position=0,unused=length-line.total;const int count=int(line.states.size());
  if(box.justify==1)position=unused;else if(box.justify==2)position=unused*.5;
  else if(box.justify==3)extra_end=std::max(0.,unused/double(std::max(1,count-1)));
  else if(box.justify==4){extra_begin=extra_end=std::max(0.,unused/double(std::max(1,count*2)));}
  else if(box.justify==5){extra_begin=extra_end=std::max(0.,unused/double(std::max(1,count+1)));}
  for(auto j:line.states){auto&s=states[j];s.margin[main][0]=extra_begin+s.margin[main][0];if(box.justify!=5)s.margin[main][1]=extra_end+s.margin[main][1];
   auto&r=result[s.index];r[main]=float(position+s.margin[main][0]);r[cross]=float(s.margin[cross][0]);position+=extent(s,main);
  }
 }
 for(auto&line:lines)for(auto j:line.states){auto&s=states[j];auto&r=result[s.index];r[cross]=float(line.position+s.margin[cross][0]);r[2]=float(s.size[0]);r[3]=float(s.size[1]);}
 for(auto&r:result){if(box.direction==1||box.direction==3){float endpoint=r[main]+r[main+2];r[main]=float(length-double(endpoint));}if(box.wrap==2){float endpoint=r[cross]+r[cross+2];r[cross]=float(cross_length-double(endpoint));}r[0]=area[0]+r[0];r[1]=area[1]+r[1];}
 return result;
}
std::vector<std::vector<Rect>> layout_tree(const std::vector<Node>&nodes,Rect area){
 if(nodes.empty())return {};
 if(nodes.size()>256)throw std::invalid_argument("Flex tree node limit");
 std::vector<int>parents(nodes.size(),0);for(std::size_t j=0;j<nodes.size();++j){const auto&n=nodes[j];if(n.children.size()!=n.items.size())throw std::invalid_argument("Flex child count");for(int child:n.children)if(child!=-1){if(child<=int(j)||std::size_t(child)>=nodes.size()||++parents[child]!=1)throw std::invalid_argument("Flex tree topology");}}
 for(std::size_t j=1;j<nodes.size();++j)if(parents[j]!=1)throw std::invalid_argument("Flex unreachable node");
 std::vector<std::vector<Rect>>output(nodes.size());
 auto visit=[&](auto&&self,std::size_t index,Rect bounds)->void{const auto&node=nodes[index];output[index]=layout(node.box,node.items,bounds);for(std::size_t j=0;j<node.children.size();++j)if(node.children[j]>=0)self(self,std::size_t(node.children[j]),output[index][j]);};
 visit(visit,0,area);return output;
}
} // namespace mfrk::ui::flex
