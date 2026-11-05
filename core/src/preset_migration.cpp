#include "mfrk/preset_migration.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace mfrk::preset {
Version parse_version(std::string_view text) {
 Version result{};
 if(text.empty())return result;
 for(unsigned index=0;index<3 && !text.empty();++index) {
  const auto end=text.find('.');const auto part=text.substr(0,end);
  if(part.empty())throw std::invalid_argument("Empty version component");
  std::uint32_t number=0;
  for(char c:part){if(c<'0'||c>'9'||number>6553u)throw std::invalid_argument("Unsupported version component");number=number*10u+static_cast<unsigned>(c-'0');}
  if(number>65535u)throw std::invalid_argument("Unsupported version component");
  result[index]=static_cast<std::int32_t>(number);
  if(end==std::string_view::npos)break;
  text.remove_prefix(end+1);
 }
 return result;
}
Value convert_discrete(const Definition& definition,Version version,Value value,bool reset_flag,float reset_normalized) {
 if(!value.present)return value;
 if(reset_flag)value.normalized=reset_normalized;
 if(definition.records.empty()||definition.records.size()>16||definition.current_item_count==0||definition.current_item_count>256)throw std::invalid_argument("Unsupported migration descriptor bounds");
 for(const auto& record:definition.records)if(record.labels.empty()||record.labels.size()>256)throw std::invalid_argument("Unsupported migration label count");
 if(!std::isfinite(value.normalized)||value.normalized<-.1f||value.normalized>1.1f)throw std::invalid_argument("Unsupported migration normalized value");
 std::string_view old_label;
 for(const auto& record:definition.records)if(record.version<=version){
  const float scaled=static_cast<float>(record.labels.size())*value.normalized;
  const auto raw=static_cast<std::int32_t>(scaled);
  const auto index=std::clamp(raw,0,static_cast<std::int32_t>(record.labels.size()-1));
  old_label=record.labels[static_cast<std::size_t>(index)];
 }
 value.normalized=definition.default_normalized;
 const auto latest=definition.records.back().labels;
 for(std::size_t index=0;index<latest.size();++index)if(latest[index]==old_label){
  const float denominator=static_cast<float>(definition.current_item_count-1);
  value.normalized=denominator==0 ? 0.0f : static_cast<float>(index)/denominator;
 }
 return value;
}
NumericWrite select_numeric(std::uint32_t id,bool present,std::uint32_t saved,std::uint32_t fallback,bool reset,std::uint32_t reset_bits){
 return {id,2,1,4,reset?reset_bits:present?saved:fallback};
}
}
