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
 if(source.buckets<8||source.buckets>512||(source.buckets&(source.buckets-1))!=0||source.iteration.size()>128||source.iteration.size()>source.buckets)return std::nullopt;
 std::vector<std::uint64_t> result;result.reserve(source.iteration.size());const auto mask=source.buckets-1;
 for(auto identity:source.iteration){
  if(!identity||std::find(result.begin(),result.end(),identity)!=result.end())return std::nullopt;
  const auto bucket=hash(identity)&mask;
  const auto position=std::find_if(result.begin(),result.end(),[&](auto other){return (hash(other)&mask)==bucket;});
  result.insert(position,identity);
 }
 return result;
}
}
