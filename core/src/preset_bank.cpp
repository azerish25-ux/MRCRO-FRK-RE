#include "mfrk/preset_bank.hpp"
namespace mfrk::preset_bank {
namespace {
bool valid_bank(const Bank& bank){
 if(bank.internal.size()>16384||bank.positive.size()>16384)return false;
 for(std::size_t i=0;i<bank.internal.size();++i)if(bank.internal[i].id!=(0x80000000u|i))return false;
 for(std::size_t i=0;i<bank.positive.size();++i){const auto* r=bank.positive.at(i);if(!r||r->id()!=i)return false;}
 return true;
}
std::uint32_t scalar(std::span<const std::uint8_t> bytes){std::uint32_t bits=0;for(unsigned i=0;i<4;++i)bits|=std::uint32_t(bytes[i])<<(8*i);return bits;}
}
bool stage(Bank& bank,std::span<const preset_program::Write> writes){
 if(!valid_bank(bank)||writes.size()>bank.internal.size()+bank.positive.size())return false;
 std::vector<bool> internal(bank.internal.size()),positive(bank.positive.size());
 for(const auto& w:writes){const bool is_internal=(w.wrapper_id&0x80000000u)!=0;auto& seen=is_internal?internal:positive;const auto index=w.wrapper_id&0x7fffffffu;
  if(index>=seen.size()||seen[index]||!w.dirty||w.payload.size()>512)return false;
  if(w.tag==1){if(w.payload.size()!=4)return false;if(!is_internal&&(scalar(w.payload)&0x7f800000u)==0x7f800000u)return false;}
  else if(!is_internal||(w.tag!=2&&w.tag!=3)||(w.tag==3&&w.payload.size()>16)||(w.tag==2&&w.payload.size()<=16))return false;
  seen[index]=true;
 }
 // Allocate every incoming typed value before touching receiver state. The
 // subsequent native-style moves transfer heaps and reset each source to zero.
 std::vector<Value> incoming;incoming.reserve(writes.size());for(const auto& w:writes)incoming.push_back(w.tag==1?Value::scalar(scalar(w.payload)):Value::data(w.payload));
 for(std::size_t i=0;i<writes.size();++i){const auto id=writes[i].wrapper_id;if(id&0x80000000u)bank.internal[id&0x7fffffffu].apply_prepared(std::move(incoming[i]));else if(!bank.positive.at(id)->apply_prepared(std::move(incoming[i])))std::terminate();}
 return true;
}
std::optional<std::vector<preset_program::Command>> notifications(const Bank& bank,std::span<const std::uint32_t> internal,std::span<const std::uint32_t> positive){
 if(!valid_bank(bank)||internal.size()!=bank.internal.size()||positive.size()!=bank.positive.size())return std::nullopt;
 std::vector<preset_program::Write> values;values.reserve(bank.internal.size()+bank.positive.size());
 auto append=[&](std::uint32_t id,const Value& value,bool dirty,std::optional<std::uint64_t> cache){const auto bytes=value.bytes();values.push_back({id,value.tag(),{bytes.begin(),bytes.end()},dirty,cache});};
 // Unsaved runtime payloads (including FeedbackData) remain in their bank and
 // do not enter the ordinary preset notification program.
 for(std::size_t i=0;i<internal.size();++i)if((internal[i]&2)!=0&&(internal[i]&16)==0){const auto& r=bank.internal[i];append(r.id,r.value,r.dirty,std::nullopt);}
 for(std::size_t i=0;i<positive.size();++i)if((positive[i]&2)!=0&&(positive[i]&16)==0){const auto* r=bank.positive.at(i);append(r->id(),r->value(),r->dirty(),r->cached_bits());}
 return preset_program::notifications_group0(internal,positive,values);
}
}
