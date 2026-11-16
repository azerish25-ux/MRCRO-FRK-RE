#pragma once
#include "audio_sample_archive_codec.hpp"
#include "mfrk/preset_migration.hpp"
#include <string_view>
namespace mfrk::preset_detail::mfv_wt_migration {
using Blob=std::vector<std::uint8_t>;
struct Result { std::int32_t metadata_word_194; bool changed; std::optional<Blob> blob; };
inline std::string_view leaf(std::string_view path){
 if(path=="/"||path=="\\")return path;
 if(!path.empty()&&(path.back()=='/'||path.back()=='\\'))return ".";
 const auto p=path.find_last_of("/\\");return p==std::string_view::npos?path:path.substr(p+1);
}
inline std::string_view stem(std::string_view path){
 auto name=leaf(path);if(name=="."||name=="..")return name;
 const auto p=name.find_last_of('.');return p==std::string_view::npos?name:name.substr(0,p);
}
inline std::uint64_t hash(std::string_view name){std::uint64_t h=5381;for(unsigned char c:name)h=h*33+c;return h;}
// Maincontext0, known Osc1WavetableObject only, blob<=512, ASCII paths and fixed native
// release archive profile. metadata_word_194 is deliberately opaque, not an
// invented class7 semantic. Only that word/known field are returned; unknown
// metadata/other fields belong to the caller and must remain untouched.
// NonASCII, foreign archive profiles, malformed path strings, bare drives/UNC
// roots and outputs>=256 bytes are outside this admitted implementation.
inline std::optional<Result> migrate(std::int32_t word,mfrk::preset::Version version,
                                    std::optional<std::span<const std::uint8_t>> blob){
 Result result{word==49?49:-1,word!=(word==49?49:-1),std::nullopt};
 if(!blob)return result;
 if(blob->size()>512)return std::nullopt;
 result.blob=Blob(blob->begin(),blob->end());
 if(blob->empty()||blob->front()==0){result.blob.reset();result.changed=true;return result;}
 auto decoded=audio_sample_archive::decode(*blob);if(!decoded)return std::nullopt;
 auto sample=decoded->sample;const auto length=static_cast<std::size_t>(sample.path_length);
 if(sample.raw_path[length]!=0)return std::nullopt;
 for(std::size_t i=0;i<length;++i)if(sample.raw_path[i]==0||sample.raw_path[i]>=128)return std::nullopt;
 const std::string_view path(reinterpret_cast<const char*>(sample.raw_path.data()),length);
 if(path.starts_with("//")||path.starts_with("\\\\"))return std::nullopt;
 if(path.size()==3&&path[1]==':'&&(path[2]=='/'||path[2]=='\\'))return std::nullopt;
 const auto first=stem(path);
 if(first.find(':')!=std::string_view::npos)return std::nullopt;
 if(first.empty()){result.blob.reset();result.changed=true;return result;}
 constexpr mfrk::preset::Version threshold{2,80,0};
 if(version>=threshold)return result;
 auto name=first;const auto key=hash(stem(first));
 bool known=false;
 if(key==0xd0a96f2e3f79ull){name="Growl 1";known=true;}
 if(key==0x1ae5d754f62ee2ull){name="Growl 2";known=true;}
 if(key==0x27a00afc06ffb593ull){name="FM Filter Woo";known=true;}
 (void)known;
 name=stem(name);if(name=="/"||name=="\\")name={};
 std::string canonical="Factory/WT/Factory/";canonical+=name;canonical+=".raw";
 if(canonical.size()>=256)return std::nullopt;
 sample.raw_path.fill(0);std::copy(canonical.begin(),canonical.end(),sample.raw_path.begin());sample.path_length=canonical.size();
 result.blob=audio_sample_archive::encode(sample);result.changed=true;
 return result;
}
}
