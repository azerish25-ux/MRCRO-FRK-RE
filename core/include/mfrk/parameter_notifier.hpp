#pragma once
#include "mfrk/listener_registry.hpp"
#include "mfrk/parameter_dispatch.hpp"
#include <array>
#include <exception>
#include <memory>
namespace mfrk::parameter_dispatch {
class ParameterNotifier;
// Routes the listener's reverse-owner cleanup to real, live notifier owners.
// This directory outlives all notifiers/listeners using it. No unknown-token
// fallback is installed: that would conceal a broken ownership relationship.
class NotifierDirectory final:public ListenerRegistryBoundary {
 friend class ParameterNotifier;
 std::map<std::uint64_t,ParameterNotifier*> notifiers_;
 void attach(std::uint64_t identity,ParameterNotifier& notifier){
  if(!identity||!notifiers_.emplace(identity,&notifier).second)throw std::invalid_argument("Unique notifier identity required");
 }
 void detach(std::uint64_t identity)noexcept{notifiers_.erase(identity);}
public:
 NotifierDirectory()=default;
 NotifierDirectory(const NotifierDirectory&)=delete;
 NotifierDirectory& operator=(const NotifierDirectory&)=delete;
 void remove(std::uint64_t notifier,std::uint32_t id,std::uint64_t listener)noexcept override;
};
namespace detail {
class OwnedListenerSet {
 struct Member {std::uint64_t identity;ListenerRegistry* listener;};
 using Rows=std::list<Member>;using Iterator=Rows::iterator;
 Rows rows_;std::vector<Iterator> first_;std::map<std::uint64_t,Iterator> index_;
 ListenerSet view_;bool dirty_=true;
 static std::uint64_t hash(std::uint64_t identity)noexcept{
  std::uint64_t value=0xcbf29ce484222325ull;
  for(unsigned i=0;i<8;++i){value^=(identity>>(i*8))&255u;value*=0x100000001b3ull;}return value;
 }
 void rehash(std::size_t count){
  std::vector<Iterator> next(count,rows_.end());
  for(auto it=rows_.begin();it!=rows_.end();){auto current=it++;auto& first=next[hash(current->identity)&(count-1)];if(first!=rows_.end())rows_.splice(first,rows_,current);first=current;}
  first_.swap(next);
 }
public:
 OwnedListenerSet():first_(8,rows_.end()){}
 OwnedListenerSet(const OwnedListenerSet&)=delete;
 OwnedListenerSet& operator=(const OwnedListenerSet&)=delete;
 bool insert(ListenerRegistry& listener){
  const auto identity=listener.identity();if(index_.contains(identity))return false;
  if(rows_.size()>=16384)throw std::length_error("Listener set profile");
  const auto required=rows_.size()+1;
  if(required>first_.size())rehash(std::bit_ceil(first_.size()<512?first_.size()*8:required));
  auto& first=first_[hash(identity)&(first_.size()-1)];const auto added=rows_.insert(first,{identity,&listener});index_.emplace(identity,added);first=added;dirty_=true;return true;
 }
 ListenerRegistry* find(std::uint64_t identity)const noexcept{const auto at=index_.find(identity);return at==index_.end()?nullptr:at->second->listener;}
 bool erase(std::uint64_t identity){
  const auto at=index_.find(identity);if(at==index_.end())return false;const auto current=at->second;const auto bucket=hash(identity)&(first_.size()-1);
  if(first_[bucket]==current){const auto next=std::next(current);first_[bucket]=next!=rows_.end()&&(hash(next->identity)&(first_.size()-1))==bucket?next:rows_.end();}
  rows_.erase(current);index_.erase(at);dirty_=true;return true;
 }
 bool empty()const noexcept{return rows_.empty();}
 const ListenerSet& view(){
  if(dirty_){view_.buckets=static_cast<std::uint32_t>(first_.size());view_.iteration.clear();view_.iteration.reserve(rows_.size());for(const auto& row:rows_)view_.iteration.push_back(row.identity);dirty_=false;}return view_;
 }
 void forget_from_listeners(std::uint64_t notifier)noexcept{for(const auto& row:rows_)row.listener->forget_notifier(notifier);}
};
}
// Owned source ParameterNotifier registration/removal/primary dispatch/cleanup.
// Forward key storage is portable; set hash/list order and reverse-ownership
// effects follow the native routines. Three parts, each primary and alternate.
// Serial access, successful allocation, valid listener/callback lifetimes,
// no registration after close, <=16384 listeners/key and no exceptions during
// cleanup are required. Concrete callback families and GUI services are supplied
// by the application; this object never substitutes a scalar parameter store.
class ParameterNotifier {
 using Map=std::map<std::uint32_t,std::unique_ptr<detail::OwnedListenerSet>>;
 std::array<Map,6> maps_;
 std::uint64_t identity_;NotifierDirectory& directory_;State state_{};bool destroying_=false;
 static std::size_t part(std::uint32_t id){
  const auto group=(id&0x7fffffffu)/1000000u;if(group>=3)throw std::invalid_argument("Notifier part profile");return group;
 }
 void clear(Map& map)noexcept{for(auto& entry:map)entry.second->forget_from_listeners(identity_);map.clear();}
public:
 ParameterNotifier(std::uint64_t identity,NotifierDirectory& directory):identity_(identity),directory_(directory){directory_.attach(identity_,*this);}
 ParameterNotifier(const ParameterNotifier&)=delete;
 ParameterNotifier& operator=(const ParameterNotifier&)=delete;
 ~ParameterNotifier(){close();directory_.detach(identity_);}
 bool add(std::uint32_t id,ListenerRegistry* listener,bool alternate=false){
  if(!listener)return false;
  if(destroying_)throw std::logic_error("Notifier is closing");
  const auto slot=part(id)+(alternate?3:0);
  if(!listener->insert(id,identity_))return false;
  auto& set=maps_[slot][id];if(!set)set=std::make_unique<detail::OwnedListenerSet>();
  return set->insert(*listener);
 }
 bool remove(std::uint32_t id,std::uint64_t listener){
  if(destroying_)return false;
  const auto group=part(id);
  for(const auto slot:{group,group+3}){
   auto& map=maps_[slot];const auto at=map.find(id);if(at==map.end())continue;
   auto* owner=at->second->find(listener);if(!owner)continue;
   at->second->erase(listener);owner->erase(id);if(at->second->empty())map.erase(at);return true;
  }
  return false;
 }
 // Original6d16a0 dispatches the primary set. Callback must invoke the real
 // listener independently of current membership: the native clone survives
 // removal of subscriptions during a callback. Payload remains shared.
 template<class Payload,class Callback>
 bool notify(std::uint32_t id,Payload& payload,std::uint32_t origin,Callback& callback){
  (void)part(id);
  struct Adapter {
   ParameterNotifier& owner;Callback& callback;
   const ListenerSet* lookup(std::uint32_t key){return owner.members(key,false);}
   void invoke(std::uint64_t listener,std::uint32_t key,Payload& value,std::uint32_t source){callback.invoke(listener,key,value,source);}
  } adapter{*this,callback};
  return dispatch(state_,id,payload,origin,adapter);
 }
 const ListenerSet* members(std::uint32_t id,bool alternate=false){const auto slot=part(id)+(alternate?3:0);const auto at=maps_[slot].find(id);return at==maps_[slot].end()?nullptr:&at->second->view();}
 void close()noexcept{if(destroying_)return;for(unsigned group=0;group<3;++group){clear(maps_[group]);clear(maps_[group+3]);}destroying_=true;}
 bool destroying()const noexcept{return destroying_;}
 bool dispatching()const noexcept{return state_.dispatching;}
 std::uint64_t identity()const noexcept{return identity_;}
};
inline void NotifierDirectory::remove(std::uint64_t notifier,std::uint32_t id,std::uint64_t listener)noexcept{
 const auto at=notifiers_.find(notifier);if(at==notifiers_.end())std::terminate();at->second->remove(id,listener);
}
}
