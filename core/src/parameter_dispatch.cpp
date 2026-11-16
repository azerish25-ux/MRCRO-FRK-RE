#include "mfrk/parameter_dispatch.hpp"
#include <algorithm>
namespace mfrk::parameter_dispatch {
namespace {
std::uint64_t hash(std::uint64_t identity) noexcept {
 std::uint64_t value=0xcbf29ce484222325ull;
 for(unsigned i=0;i<8;++i){value^=(identity>>(8*i))&255u;value*=0x100000001b3ull;}
 return value;
}
}
std::optional<std::vector<std::uint64_t>> snapshot(const ListenerSet& source){
 if(source.buckets<8||source.buckets>65536||(source.buckets&(source.buckets-1))!=0||source.iteration.size()>16384||source.iteration.size()>source.buckets)return std::nullopt;
 auto result=source.iteration;
 std::sort(result.begin(),result.end());
 if(!result.empty()&&(result.front()==0||std::adjacent_find(result.begin(),result.end())!=result.end()))return std::nullopt;
 result.clear();
 const auto end=source.iteration.size();if(!end)return result;
 struct Link {std::size_t previous,next;};
 std::vector<Link> links(end);std::vector<std::size_t> first(source.buckets,end);
 std::size_t head=end,tail=end;const auto mask=source.buckets-1;
 // Reproduce native list insertion before the first node of a populated
 // bucket, or before the list sentinel for an empty bucket. Index links avoid
 // repeatedly moving or hashing the entire list for large real UI fanouts.
 for(std::size_t i=0;i<end;++i){
  auto& position=first[hash(source.iteration[i])&mask];
  const auto previous=position==end?tail:links[position].previous;
  links[i]={previous,position};
  if(position==end)tail=i;else links[position].previous=i;
  if(previous==end)head=i;else links[previous].next=i;
  position=i;
 }
 for(auto i=head;i!=end;i=links[i].next)result.push_back(source.iteration[i]);
 return result;
}
}
