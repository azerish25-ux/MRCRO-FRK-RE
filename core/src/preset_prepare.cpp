#include "mfrk/preset_prepare.hpp"
#include "preset_detail/mfv_wt_migration.hpp"
#include <bit>
#include <utility>
namespace mfrk::preset_prepare {
std::optional<State> from_archive(const preset_archive::View& view,std::int32_t word194){
 if(view.product!=66||view.parameters.size()>10000||view.blobs.size()>1000)return std::nullopt;
 State state;state.creator=view.creator_version;state.word194=word194;
 for(const auto& entry:view.parameters){auto bits=preset_archive::numeric_bits(entry.value);if(!bits||!state.numeric.emplace(entry.key,*bits).second)return std::nullopt;}
 for(const auto& entry:view.blobs){if(!state.data.emplace(entry.key,std::vector<std::uint8_t>(entry.value.begin(),entry.value.end())).second)return std::nullopt;}
 return state;
}
std::optional<Result> prepare(State input,std::span<const preset_program::Descriptor> internal,std::span<const preset_program::Descriptor> vst,std::span<const Versioned> versioned){
 if(internal.size()>16384||vst.size()>16384||versioned.size()>16||input.numeric.size()>10000||input.data.size()>1000)return std::nullopt;
 const auto version=preset::parse_version(input.creator);
 for(const auto& entry:versioned){auto value=input.numeric.find(entry.name);const bool present=value!=input.numeric.end();auto converted=preset::convert_discrete(entry.definition,version,{present,present?std::bit_cast<float>(value->second):0.f},(entry.flags&256)!=0,std::bit_cast<float>(entry.reset_bits));if(present)value->second=std::bit_cast<std::uint32_t>(converted.normalized);}
 const auto found=input.data.find("Osc1WavetableObject");std::optional<std::span<const std::uint8_t>> blob;if(found!=input.data.end())blob=found->second;
 auto custom=preset_detail::mfv_wt_migration::migrate(input.word194,version,blob);if(!custom)return std::nullopt;
 input.word194=custom->metadata_word_194;
 if(custom->blob)input.data.insert_or_assign("Osc1WavetableObject",std::move(*custom->blob));else input.data.erase("Osc1WavetableObject");
 if(input.creator.size()>15)return std::nullopt;
 input.creator="5.1.1.6945";
 auto writes=preset_program::prepare_group0(internal,vst,input.numeric,input.data);if(!writes)return std::nullopt;
 return Result{std::move(input),custom->changed,std::move(*writes)};
}
}
