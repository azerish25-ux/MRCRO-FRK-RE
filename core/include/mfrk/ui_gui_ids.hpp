#pragma once
#include <array>
#include <cstdint>
#include <optional>
namespace mfrk::ui {
// Source67d450/699f00 table lookup. Mode1 selects the second context table;
// every other mode selects the first. Invalid payload bytes are not exposed.
struct GuiIdTables {
 std::array<std::array<std::int32_t,75>,2> features;
 std::array<std::array<std::int32_t,204>,2> properties;
 GuiIdTables(){for(auto& table:features)table.fill(-1);for(auto& table:properties)table.fill(-1);}
 std::optional<std::uint32_t> feature(std::uint32_t mode,std::uint32_t category,std::uint32_t part)const{
  if(category>=25||part>=3)return std::nullopt;
  const auto id=features[mode==1?1:0][part*25+category];return id==-1?std::nullopt:std::optional(static_cast<std::uint32_t>(id));
 }
 std::optional<std::uint32_t> property(std::uint32_t mode,std::uint32_t category,std::uint32_t part)const{
  if(category>=68||part>=3)return std::nullopt;
  const auto id=properties[mode==1?1:0][part*68+category];return id==-1?std::nullopt:std::optional(static_cast<std::uint32_t>(id));
 }
};
}
