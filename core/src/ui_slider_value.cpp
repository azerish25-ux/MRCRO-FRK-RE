#include "mfrk/ui_slider_value.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <locale.h>
#include <limits>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
namespace mfrk::ui {
namespace {
#ifdef _WIN32
struct NumericLocale {
 HMODULE module=LoadLibraryExW(L"ucrtbase.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
 template<class F>F function(const char* name){const auto address=module?GetProcAddress(module,name):nullptr;if(!address)throw std::runtime_error("C locale runtime");return std::bit_cast<F>(address);}
 using Create=void*(__cdecl*)(int,const char*);using Free=void(__cdecl*)(void*);
 using Print=int(__cdecl*)(std::uint64_t,char*,std::size_t,const char*,void*,const void*);
 using Parse=float(__cdecl*)(const char*,char**,void*);
 Free release=function<Free>("_free_locale");Print print=function<Print>("__stdio_common_vsprintf");Parse parse=function<Parse>("_strtof_l");
 void* value=function<Create>("_create_locale")(0,"C");
 NumericLocale(){if(!value)throw std::runtime_error("C locale");}
 ~NumericLocale(){release(value);FreeLibrary(module);}
};
#else
struct NumericLocale {
 locale_t value=newlocale(LC_NUMERIC_MASK,"C",nullptr),previous=nullptr;
 NumericLocale(){if(!value)throw std::runtime_error("C locale");previous=uselocale(value);if(!previous){freelocale(value);throw std::runtime_error("thread locale");}}
 ~NumericLocale(){uselocale(previous);freelocale(value);}
};
#endif
float clamp(float value){return value>=0.f?(value<=1.f?value:1.f):0.f;}
}
float slider_to_mapped(SliderValueInfo& info,float value){
 if(info.reverse)value=1.f-value;
 value=clamp(value);
 if(info.direct)return info.curve.forward(value);
 const auto from=info.from,to=info.to;
 return info.curve.forward(value)*(to-from)+from;
}
float slider_to_normalized(SliderValueInfo& info,float value){
 if(!info.direct){const auto lower=std::min(info.from,info.to),upper=std::max(info.from,info.to);if(value<lower)value=lower;else if(value>upper)value=upper;const float span=info.to-info.from;value=span==0.f?0.f:(value-info.from)/span;}
 value=info.curve.inverse(value);if(info.reverse)value=1.f-value;
 return clamp(value);
}
float slider_format_roundtrip(float value,std::int32_t precision){
 if(!std::isfinite(value)||precision>6)throw std::invalid_argument("slider format profile");
 if(precision<0){const auto magnitude=std::abs(value);precision=magnitude<.0005f?2:magnitude<1.f?3:magnitude<10.f?2:magnitude<100.f?1:0;}
 std::int32_t scale=1;for(std::int32_t i=0;i<precision;++i)scale*=10;
 const auto factor=static_cast<float>(scale);value=std::round(value*factor)/factor;
 if(!std::isfinite(value))throw std::invalid_argument("slider scaled format profile");
 if(std::abs(value)<std::numeric_limits<float>::epsilon())value=0.f;
 char text[64];char* end=nullptr;
#ifdef _WIN32
 static const NumericLocale locale;const struct Arguments {std::int32_t precision,padding;double value;} arguments{precision,0,static_cast<double>(value)};
 const int size=locale.print(2,text,sizeof text,"%.*f",locale.value,&arguments);
#else
 NumericLocale locale;const int size=std::snprintf(text,sizeof text,"%.*f",precision,static_cast<double>(value));
#endif
 if(size<0||size>=static_cast<int>(sizeof text))throw std::invalid_argument("slider text profile");
#ifdef _WIN32
 const auto result=locale.parse(text,&end,locale.value);
#else
 const auto result=std::strtof(text,&end);
#endif
 if(end!=text+size||!std::isfinite(result))throw std::invalid_argument("slider parse profile");
 return result;
}
float slider_write(SliderValueConfig& config,SliderValueInfo& info,std::uint32_t id,float value,bool precise,SliderValueBoundary& boundary){
 if(config.reverse)value=1.f-value;
 const auto count=boundary.item_count(id);
 if(count>4096)throw std::invalid_argument("slider count profile");
 if(count>0){const auto index=std::clamp(static_cast<std::int32_t>(static_cast<float>(count)*value),0,count-1);value=count==1?0.f:static_cast<float>(index)/static_cast<float>(count-1);if(config.index_signal)boundary.index(static_cast<std::uint32_t>(index));}
 else if(info.precision!=-1){
  bool format=config.mapping_mode!=1;
  if(config.mapping_mode==2)format=id==boundary.selected_id();else if(config.mapping_mode==3)format=id!=boundary.selected_id();
  if(format){const auto mapped=slider_to_mapped(info,value);auto rounded=slider_format_roundtrip(mapped,info.precision);if(precise)rounded=slider_format_roundtrip(mapped,info.fine_precision);else if(config.offset!=0.f)rounded=config.offset+rounded;value=slider_to_normalized(info,rounded);}
 }
 boundary.set(id,value,true);
 boundary.mapped(static_cast<double>(id),static_cast<double>(slider_to_mapped(info,value)));
 if(config.normal_signal)boundary.normalized(static_cast<double>(value));
 if(config.id_signal)boundary.id_value(id,value);
 return config.reverse?1.f-value:value;
}
}
