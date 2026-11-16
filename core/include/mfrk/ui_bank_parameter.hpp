#pragma once
#include "mfrk/ui_parameter_controller.hpp"
namespace mfrk::ui {
// Borrow the actual parameter storage selected from a source bank. The caller
// first resolves the part and removes its million-ID offset (source6682f0).
// Resolve again after bank activation; never retain this view across bank
// destruction/replacement. No normalized cache or copied scalar is introduced.
class GuiBankParameterView final:public GuiParameterValue {
 const preset_bank::Value* value_;
 explicit GuiBankParameterView(const preset_bank::Value& value):value_(&value){}
public:
 static std::optional<GuiBankParameterView> resolve(const preset_bank::Bank& bank,std::uint32_t local_id){
  const auto index=local_id&0x7fffffffu;
  if(index>=1000000)return std::nullopt;
  if(local_id&0x80000000u){
   if(index>=bank.internal.size())return std::nullopt;
   const auto& receiver=bank.internal[index];
   if(receiver.id!=local_id)return std::nullopt;
   return GuiBankParameterView(receiver.value);
  }
  const auto ordinal=bank.positive.index_of(local_id);
  if(!ordinal)return std::nullopt;
  const auto* receiver=bank.positive.at(*ordinal);
  if(!receiver)return std::nullopt;
  return GuiBankParameterView(receiver->value());
 }
 const preset_bank::Value* current()override{return value_;}
};
}
