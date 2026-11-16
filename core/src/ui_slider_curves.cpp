#include "mfrk/ui_slider_curves.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace mfrk::ui {
namespace {
class Curve final:public SliderCurve {
 SliderCurveDefinition data_;
public:
 explicit Curve(SliderCurveDefinition data):data_(std::move(data)){}
 float forward(float value)override {
  const auto& p=data_.fields;
  switch(data_.kind){
   case SliderCurveKind::linear:return value;
   case SliderCurveKind::reverse:return 1.f-value;
   case SliderCurveKind::inverse:return (1.f/((p[3]-p[1])*value+p[1])-p[0])/(p[2]-p[0]);
   case SliderCurveKind::piecewise:return value<.75f?value*1.3333334f*p[0]:(value-.75f)*4.f*(1.f-p[0])+p[0];
   case SliderCurveKind::harmonic:if(value<.5f)return 1.f/((value+value)*(1.f-p[0])+p[0]);else if(value>.5f)return ((value+value)-1.f)*(p[0]-1.f)+1.f;else return 1.f;
   case SliderCurveKind::table:{
    if(value<0.f||value>1.f||!std::isfinite(value))throw std::invalid_argument("table forward profile");
    const float position=static_cast<float>(data_.table.size()-2)*value;const auto index=static_cast<std::size_t>(position);
    const auto left=data_.table[index];return (position-static_cast<float>(index))*(data_.table[index+1]-left)+left;
   }
  }
  throw std::invalid_argument("curve kind");
 }
 float inverse(float value)override {
  const auto& p=data_.fields;
  switch(data_.kind){
   case SliderCurveKind::linear:return value;
   case SliderCurveKind::reverse:return 1.f-value;
   case SliderCurveKind::inverse:return (1.f/((p[2]-p[0])*value+p[0])-p[1])/(p[3]-p[1]);
   case SliderCurveKind::piecewise:{const float result=value<p[0]?(value/p[0])*.75f:((value-p[0])/(1.f-p[0]))*.25f+.75f;return result<=0.f?0.f:result>=1.f?1.f:result;}
   case SliderCurveKind::harmonic:if(value<1.f)return (.5f/(1.f-p[0]))*(1.f/value-p[0]);else if(value>1.f)return (.5f/(p[0]-1.f))*(value-1.f)+.5f;else return .5f;
   case SliderCurveKind::table:{
    std::size_t first=0,count=data_.table.size()-1;
    while(count){const auto step=count/2,index=first+step;if(data_.table[index]<value){first=index+1;count-=step+1;}else count=step;}
    if(first==0)return 0.f;
    const float gap=data_.table[first]-data_.table[first-1];const float numerator=(static_cast<float>(first-1)*gap+value)-data_.table[first-1];
    return numerator/(static_cast<float>(data_.table.size()-2)*gap);
   }
  }
  throw std::invalid_argument("curve kind");
 }
};
}
std::unique_ptr<SliderCurve> make_slider_curve(const SliderCurveDefinition& definition){
 for(auto x:definition.fields)if(!std::isfinite(x))return nullptr;
 switch(definition.kind){
  case SliderCurveKind::linear:case SliderCurveKind::reverse:break;
  case SliderCurveKind::inverse:if(definition.fields[2]==definition.fields[0]||definition.fields[3]==definition.fields[1])return nullptr;break;
  case SliderCurveKind::piecewise:if(definition.fields[0]<=0.f||definition.fields[0]>=1.f)return nullptr;break;
  case SliderCurveKind::harmonic:if(definition.fields[0]<=0.f||definition.fields[0]==1.f)return nullptr;break;
  case SliderCurveKind::table:if(definition.table.size()<3||definition.table.size()>4096)return nullptr;for(std::size_t i=0;i<definition.table.size();++i)if(!std::isfinite(definition.table[i])||(i&&definition.table[i]<=definition.table[i-1]))return nullptr;break;
  default:return nullptr;
 }
 return std::make_unique<Curve>(definition);
}
std::unique_ptr<SliderCurve> make_slider_curve(std::string_view name,std::array<float,4> fields,std::vector<float> table){
 constexpr std::string_view names[]{".?AVLinearPotardMapping@MappingTools@Arturia@@",".?AVReversePotardMapping@MappingTools@Arturia@@",".?AVInversePotardMapping@MappingTools@Arturia@@",".?AVPiecewiseLinearPotardMapping@MappingTools@Arturia@@",".?AVHarmonicMapping@MappingTools@Arturia@@",".?AVTablePotardMapping@MappingTools@Arturia@@"};
 for(unsigned i=0;i<6;++i)if(name==names[i])return make_slider_curve({static_cast<SliderCurveKind>(i),fields,std::move(table)});
 return nullptr;
}
}
