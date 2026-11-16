#pragma once
#include <bit>
#include <cstdint>
#include <iterator>
#include <list>
#include <map>
#include <stdexcept>
#include <vector>
namespace mfrk::parameter_dispatch {
struct Registration {std::uint32_t id;std::uint64_t notifier;bool operator==(const Registration&)const=default;};
struct ListenerRegistryBoundary {
 virtual ~ListenerRegistryBoundary()=default;
 virtual void remove(std::uint64_t notifier,std::uint32_t id,std::uint64_t listener)noexcept=0;
};
// Source IParamListener reverse ownership map:64b0d0/654740/5933f0/64f880,
// guarded reverse erase in6d1510, and64f800 cleanup. The supplied notifier
// service must outlive this owner. Identity is explicit and nonzero; notifier
// tokens are borrowed, not owned objects or callable native addresses.
// Serial access, load factor1, count<=2^24, successful allocation, no insertion
// during cleanup and nonthrowing callbacks. This is not the notifier's own map.
class ListenerRegistry {
 using Rows=std::list<Registration>;
 using Iterator=Rows::iterator;
 Rows rows_;
 std::vector<Iterator> first_;
 std::map<std::uint32_t,Iterator> index_;
 std::uint64_t identity_;
 ListenerRegistryBoundary& boundary_;
 bool destroying_=false;
 static std::uint64_t hash(std::uint32_t key)noexcept{
  std::uint64_t value=0xcbf29ce484222325ull;
  for(unsigned i=0;i<4;++i){value^=(key>>(i*8))&255u;value*=0x100000001b3ull;}
  return value;
 }
 void rehash(std::size_t count){
  std::vector<Iterator> next(count,rows_.end());
  for(auto it=rows_.begin();it!=rows_.end();){
   auto current=it++;
   auto& first=next[hash(current->id)&(count-1)];
   if(first!=rows_.end())rows_.splice(first,rows_,current);
   first=current;
  }
  first_.swap(next);
 }
public:
 ListenerRegistry(std::uint64_t identity,ListenerRegistryBoundary& boundary)
 :first_(8,rows_.end()),identity_(identity),boundary_(boundary){if(!identity)throw std::invalid_argument("Listener identity required");}
 ListenerRegistry(const ListenerRegistry&)=delete;
 ListenerRegistry& operator=(const ListenerRegistry&)=delete;
 ~ListenerRegistry(){close();}
 bool insert(std::uint32_t id,std::uint64_t notifier){
  if(destroying_)throw std::logic_error("Listener is closing");
  if(index_.contains(id))return false;
  if(rows_.size()>=0x1000000u)throw std::length_error("Listener registration profile");
  const auto required=rows_.size()+1;
  if(required>first_.size())rehash(std::bit_ceil(first_.size()<512?first_.size()*8:required));
  auto& first=first_[hash(id)&(first_.size()-1)];
  const auto inserted=rows_.insert(first,{id,notifier});
  index_.emplace(id,inserted);first=inserted;return true;
 }
 std::uint64_t find(std::uint32_t id)const noexcept{
  const auto at=index_.find(id);return at==index_.end()?0:at->second->notifier;
 }
 bool erase(std::uint32_t id){
  if(destroying_)return false;
  const auto at=index_.find(id);if(at==index_.end())return false;
  const auto current=at->second;const auto bucket=hash(id)&(first_.size()-1);
  if(first_[bucket]==current){
   const auto next=std::next(current);
   first_[bucket]=next!=rows_.end()&&(hash(next->id)&(first_.size()-1))==bucket?next:rows_.end();
  }
  rows_.erase(current);index_.erase(at);return true;
 }
 // Source64f920: a notifier being cleared severs every reverse association
 // with itself. It must not call that notifier again while doing so.
 void forget_notifier(std::uint64_t notifier){
  if(destroying_)return;
  for(auto it=rows_.begin();it!=rows_.end();){
   const auto current=it++;
   if(current->notifier==notifier)erase(current->id);
  }
 }
 void close()noexcept{
  if(destroying_)return;
  destroying_=true;
  for(const auto& row:rows_)if(row.notifier)boundary_.remove(row.notifier,row.id,identity_);
  index_.clear();rows_.clear();std::vector<Iterator>().swap(first_);
 }
 bool destroying()const noexcept{return destroying_;}
 std::size_t size()const noexcept{return rows_.size();}
 std::size_t bucket_count()const noexcept{return first_.size();}
 const Rows& registrations()const noexcept{return rows_;}
};
}
