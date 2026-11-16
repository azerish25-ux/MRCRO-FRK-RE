#pragma once
#include <cstdint>
namespace mfrk::ui {
struct SliderCurve {
 virtual ~SliderCurve()=default;
 virtual float forward(float normalized)=0;
 virtual float inverse(float mapped)=0;
};
struct SliderValueInfo {SliderCurve& curve;float from,to;std::int32_t precision,fine_precision;bool direct,reverse;};
struct SliderValueConfig {bool reverse;std::int32_t mapping_mode;float offset;bool index_signal,normal_signal,id_signal;};
struct SliderValueBoundary {
 virtual ~SliderValueBoundary()=default;
 virtual std::int32_t item_count(std::uint32_t)=0;
 virtual std::uint32_t selected_id()=0;
 virtual void index(std::uint32_t)=0;
 virtual void set(std::uint32_t,float,bool)=0;
 virtual void mapped(double id,double value)=0;
 virtual void normalized(double)=0;
 virtual void id_value(std::uint32_t,float)=0;
};
float slider_to_mapped(SliderValueInfo&,float normalized);
float slider_to_normalized(SliderValueInfo&,float mapped);
// Source 649f70 plus strtof: finite input/scaled result, precision0..6 or
// negative auto-precision. Uses an isolated C numeric locale, no global change.
float slider_format_roundtrip(float,std::int32_t precision);
// Source c45ec0 with stable supplied curve and GUI services. Positive enum counts
// <=4096, nonpositive counts continuous,
// finite values/intermediates and representable CVTT. Returns view-space value;
// setter/signals use parameter space. Curves, GUI lookup and actual controller
// delivery belong to supplied boundaries. Serial/no-exception valid profile.
float slider_write(SliderValueConfig&,SliderValueInfo&,std::uint32_t id,
 float view_value,bool precise,SliderValueBoundary&);
}
