#pragma once
#include "mfrk/preset_program.hpp"
#include <array>
#include <algorithm>
#include <atomic>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
namespace mfrk::preset_bank {
struct StoreTestAccess;
// Native logical ParamValue1/2/3 semantics, 32-byte owning representation.
// Successful allocations; nonalias/distinct copy inputs, data<=8192. Scalar
// bits are opaque. Not a native-pointer wire format or general tag0 model.
class Value {
 std::uint32_t tag_{1},reserved_{};std::array<std::uint8_t,16> raw_{};std::uint64_t length_{4};
 inline static thread_local void(*free_observer_)(const void*)=nullptr;
 friend struct StoreTestAccess;
 std::uint8_t* heap()const noexcept{std::uint8_t*p;std::memcpy(&p,raw_.data(),8);return p;}
 std::uint64_t allocation_size()const noexcept{std::uint64_t n;std::memcpy(&n,raw_.data()+8,8);return n;}
 void release_heap()noexcept{if(tag_==2){auto*p=heap();if(p){if(free_observer_)free_observer_(p);std::free(p);}}}
 void assign_data(std::span<const std::uint8_t> bytes){
  if(bytes.size()>8192)throw std::invalid_argument("data admission");
  if(bytes.size()<=16){release_heap();raw_.fill(0);if(!bytes.empty())std::memcpy(raw_.data(),bytes.data(),bytes.size());tag_=3;length_=bytes.size();return;}
  std::uint8_t*p=nullptr;
  bool reuse=tag_==2&&allocation_size()==bytes.size();
  if(reuse)p=heap();
  else{release_heap();tag_=1;p=static_cast<std::uint8_t*>(std::malloc(bytes.size()));if(!p)throw std::bad_alloc();std::memcpy(raw_.data(),&p,8);const std::uint64_t n=bytes.size();std::memcpy(raw_.data()+8,&n,8);}
  std::memcpy(p,bytes.data(),bytes.size());tag_=2;length_=bytes.size();
 }
 void assign_scalar(std::uint32_t bits)noexcept{release_heap();std::memcpy(raw_.data(),&bits,4);tag_=1;length_=4;}
 void move_from(Value& other)noexcept{
  const auto type=other.tag_;const auto size=other.length_;
  if(type==2){const auto contents=other.raw_;std::uint64_t zero=0;std::memcpy(other.raw_.data(),&zero,8);release_heap();raw_=contents;tag_=2;}
  else if(type==3){const auto contents=other.raw_;release_heap();raw_=contents;tag_=3;}
  else assign_scalar(other.scalar_bits());
  length_=size;other.tag_=1;other.length_=4;const std::uint32_t zero=0;std::memcpy(other.raw_.data(),&zero,4);
 }
public:
 Value()=default;
 ~Value(){release_heap();}
 static Value scalar(std::uint32_t bits){Value v;v.assign_scalar(bits);return v;}
 static Value data(std::span<const std::uint8_t> bytes){Value v;v.assign_data(bytes);return v;}
 Value(const Value&v):Value(){if(v.tag_==1)assign_scalar(v.scalar_bits());else assign_data(v.bytes());}
 Value&operator=(const Value&v){if(this!=&v){if(v.tag_==1)assign_scalar(v.scalar_bits());else{
  assign_data(v.bytes());
 }}return*this;}
 Value(Value&&v)noexcept:Value(){move_from(v);}
 Value&operator=(Value&&v)noexcept{if(this!=&v)move_from(v);return*this;}
 std::uint32_t tag()const noexcept{return tag_;}
 std::uint32_t scalar_bits()const{if(tag_!=1)throw std::logic_error("not scalar");std::uint32_t bits;std::memcpy(&bits,raw_.data(),4);return bits;}
 std::span<const std::uint8_t>bytes()const noexcept{return{tag_==2?heap():raw_.data(),static_cast<std::size_t>(length_)};}
 const void*data_address()const noexcept{return bytes().data();}
};
static_assert(sizeof(void*)==8&&sizeof(Value)==32);
class Receiver;
class Ref {
 Receiver*p_{};explicit Ref(Receiver*p):p_(p){}friend class Receiver;
public:
 Ref()=default;static Ref retain(Receiver*p);~Ref();Ref(const Ref&);Ref&operator=(const Ref&);Ref(Ref&&v)noexcept:p_(std::exchange(v.p_,nullptr)){}Ref&operator=(Ref&&)noexcept;void reset()noexcept;
 Receiver*get()const noexcept{return p_;}Receiver*operator->()const noexcept{return p_;}explicit operator bool()const noexcept{return p_!=nullptr;}
};
class Receiver {
 std::atomic<std::int32_t>refs_{1};std::uint32_t id_;Value value_;std::uint64_t cached_;bool dirty_{};std::function<void(std::uint32_t)>on_destroy_;
 friend class Ref;friend struct StoreTestAccess;
 Receiver(std::uint32_t id,Value value,std::uint64_t cache,std::function<void(std::uint32_t)>audit):id_(id),value_(std::move(value)),cached_(cache),on_destroy_(std::move(audit)){if(value_.tag()!=1)throw std::invalid_argument("positive scalar admission");}
 ~Receiver(){if(on_destroy_)on_destroy_(id_);}
 void add_ref()noexcept{refs_.fetch_add(1);}
 void release()noexcept{if(refs_.fetch_sub(1)==1){refs_.store(-1000);delete this;}}
public:
 static Ref make(std::uint32_t id,Value value,std::uint64_t cache,std::function<void(std::uint32_t)>audit={}){return Ref(new Receiver(id,std::move(value),cache,std::move(audit)));}
 std::uint32_t id()const noexcept{return id_;}const Value&value()const noexcept{return value_;}std::uint64_t cached_bits()const noexcept{return cached_;}bool dirty()const noexcept{return dirty_;}
 // Validated loader6cc510 write, before later Arturia callback delivery. This
 // finite-positive profile includes native64a930/64b030 cache clamp only.
 bool apply_prepared(Value&&incoming){if(incoming.tag()!=1)return false;const auto bits=incoming.scalar_bits();if((bits&0x7f800000u)==0x7f800000u)return false;const float x=std::bit_cast<float>(bits);const double next=x<=0?0.0:x>=1?1.0:static_cast<double>(x);const double old=std::bit_cast<double>(cached_);if(!std::isfinite(old))return false;dirty_=true;value_=std::move(incoming);if(next!=old)cached_=std::bit_cast<std::uint64_t>(next);return true;}
};
inline Ref Ref::retain(Receiver*p){if(p)p->add_ref();return Ref(p);}
inline Ref::~Ref(){reset();}
inline void Ref::reset()noexcept{if(auto*p=std::exchange(p_,nullptr))p->release();}
inline Ref::Ref(const Ref&v):p_(v.p_){if(p_)p_->add_ref();}
inline Ref&Ref::operator=(const Ref&v){if(this!=&v){auto*p=v.p_;if(p)p->add_ref();reset();p_=p;}return*this;}
inline Ref&Ref::operator=(Ref&&v)noexcept{if(this!=&v){reset();p_=std::exchange(v.p_,nullptr);}return*this;}
// Serial, unique-ID construction profile. The native positive owner is an
// optional vector object plus ID->ordinal tree, not a guessed shared_ptr.
class PositiveContainer {
 std::unique_ptr<std::vector<Ref>>rows_;std::map<std::uint32_t,std::size_t>index_;
public:
 PositiveContainer()=default;PositiveContainer(const PositiveContainer&)=delete;PositiveContainer&operator=(const PositiveContainer&)=delete;~PositiveContainer(){clear();}
 bool append(Ref&&v){if(!v||index_.contains(v->id())||size()>=16384)return false;if(!rows_){rows_=std::make_unique<std::vector<Ref>>();rows_->reserve(10);}if(rows_->size()==rows_->capacity())rows_->reserve(std::max(rows_->size()+1,rows_->capacity()+rows_->capacity()/2));index_.emplace(v->id(),rows_->size());rows_->push_back(std::move(v));return true;}
 std::size_t size()const noexcept{return rows_?rows_->size():0;}std::size_t capacity()const noexcept{return rows_?rows_->capacity():0;}
 std::optional<std::size_t>index_of(std::uint32_t id)const{auto i=index_.find(id);return i==index_.end()?std::nullopt:std::optional(i->second);}
 std::size_t index_size()const noexcept{return index_.size();}
 Receiver*at(std::size_t i)const{if(!rows_||i>=rows_->size())return nullptr;auto hold=Ref::retain((*rows_)[i].get());return hold.get();}
 void clear()noexcept{if(rows_){
  for(auto&v:*rows_)v.reset();
  rows_->clear();}index_.clear();}
 void swap_contents(PositiveContainer&v)noexcept{rows_.swap(v.rows_);
  index_.swap(v.index_);
 }
};
struct InternalReceiver {std::uint32_t id;Value value;bool dirty{};void apply_prepared(Value&&v){dirty=true;value=std::move(v);}};
struct Bank {std::vector<InternalReceiver>internal;PositiveContainer positive;};
class BankPair {
 Bank active_,prepared_;
public:
 Bank&active()noexcept{return active_;}Bank&prepared()noexcept{return prepared_;}
 // Exact value/ownership stage6d78b0 GROUP0: does not clear/reconstruct rows,
 // notify, copy defaults, swap outer group metadata or transfer listeners.
 void activate_main()noexcept{
  active_.internal.swap(prepared_.internal);
  active_.positive.swap_contents(prepared_.positive);
 }
};
// Supplied initialized group-0 banks, canonical prepared writes <=512 bytes.
// Does not construct descriptors or receivers. Unselected state and externally
// retained positive receivers remain owned by their original bank/context.
// Validation/allocation precede mutation. Runs off the audio thread.
bool stage(Bank&,std::span<const preset_program::Write>);
std::optional<std::vector<preset_program::Command>> notifications(const Bank&,
 std::span<const std::uint32_t> internal_flags,std::span<const std::uint32_t> vst_flags);

}
