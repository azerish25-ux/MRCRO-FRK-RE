#pragma once
#include <cmath>
#include "mfrk/ui_scene_prefix.hpp"
#include "ui_scene_fixture_reader.hpp"
namespace mfrk::ui::scene::test {
inline std::string string(Reader&r){auto n=r.u32();if(n>1024)throw std::runtime_error("string bound");std::string value;value.reserve(n);while(n--)value.push_back(char(r.u8()));return value;}
inline Attributes attrs(Reader&r){auto n=r.u32();if(n>64)throw std::runtime_error("attribute bound");Attributes a;while(n--){auto key=string(r),value=string(r);a.emplace_back(std::move(key),std::move(value));}return a;}
inline Constants numbers(Reader&r){auto n=r.u32();if(n>256)throw std::runtime_error("constant bound");Constants a;while(n--){auto key=string(r);auto value=r.f64();if(!std::isfinite(value)||!a.emplace(std::move(key),value).second)throw std::runtime_error("invalid numeric constant");}return a;}
inline StringConstants strings(Reader&r){auto n=r.u32();if(n>256)throw std::runtime_error("constant bound");StringConstants a;while(n--){auto key=string(r),value=string(r);if(!a.emplace(std::move(key),std::move(value)).second)throw std::runtime_error("duplicate string constant");}return a;}
}
