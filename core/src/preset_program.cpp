#include "mfrk/preset_program.hpp"
#include "mfrk/preset_migration.hpp"
#include <bit>
#include <cmath>
namespace mfrk::preset_program {
std::optional<std::vector<Write>> prepare_group0(std::span<const Descriptor> internal,std::span<const Descriptor> vst,const Numeric& numeric,const Data& data){
 if(internal.size()>16384||vst.size()>16384||numeric.size()>10000||data.size()>1000)return std::nullopt;
 std::vector<Write> writes;
 auto append=[&](std::span<const Descriptor> descriptors,bool positive){
  for(std::size_t i=0;i<descriptors.size();++i){const auto& d=descriptors[i];if((d.flags&2)!=2&&(d.flags&256)==0)continue;
   if(d.name.empty()&&(!numeric.empty()||!data.empty()))return false;
   Write w{static_cast<std::uint32_t>(i)|(positive?0u:0x80000000u),0,{},true,std::nullopt};
   if(d.default_tag==1){
    if(d.fallback.size()!=4)return false;std::uint32_t fallback=0;for(unsigned b=0;b<4;++b)fallback|=std::uint32_t(d.fallback[b])<<(b*8);
    const auto saved=numeric.find(d.name);const bool present=saved!=numeric.end();const auto applied=preset::apply_numeric({present?saved->second:0,fallback,d.reset_bits,0,1,present,(d.flags&256)!=0,(d.flags&2048)!=0,false});
    w.tag=1;for(unsigned b=0;b<4;++b)w.payload.push_back(static_cast<std::uint8_t>(applied.bits>>(b*8)));
    if(positive){const float value=std::bit_cast<float>(applied.bits);if(!std::isfinite(value))return false;const double cache=value<=0?0.0:value>=1?1.0:static_cast<double>(value);w.cached_normalized_double_bits=std::bit_cast<std::uint64_t>(cache);}
   }else{
    if(positive||(d.default_tag!=2&&d.default_tag!=3)||(d.default_tag==3&&d.fallback.size()>16))return false;
    const auto saved=data.find(d.name);std::optional<std::span<const std::uint8_t>> payload;if(saved!=data.end())payload=saved->second;
    auto applied=preset::apply_data(d.fallback,payload,(d.flags&256)!=0,(d.flags&512)!=0);if(!applied)return false;w.tag=applied->tag;w.payload=std::move(applied->payload);
   }
   writes.push_back(std::move(w));
  }return true;
 };
 if(!append(internal,false)||!append(vst,true))return std::nullopt;return writes;
}
}
