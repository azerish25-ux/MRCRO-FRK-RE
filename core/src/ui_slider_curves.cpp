#include "mfrk/ui_slider_curves.hpp"
#include "ui_master_power.hpp"
#include "ui_exponential_math.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <stdexcept>
namespace mfrk::ui {
namespace {
constexpr std::array<std::uint32_t,21> envelope_time_bits{
 0x00000000,0x39903807,0x3a89a9d8,0x3b0e947c,0x3b68b780,0x3bb44609,0x3c045652,
 0x3c436ed5,0x3c8fcf24,0x3cd26330,0x3d1af94c,0x3d4ae903,0x3d9faf7e,0x3dbcedc3,
 0x3e19cbc0,0x3e4a0392,0x3e989e33,0x3ec27902,0x3f088b0c,0x3f2f8fd7,0x3f800000};
constexpr float envelope_time(std::size_t i){return std::bit_cast<float>(envelope_time_bits[i]);}
constexpr std::array<std::uint32_t,10> mixer_time_bits{
 0x00000000,0x3ab85b9f,0x3b8975b8,0x3c0cb1b6,0x3c9d78ee,
 0x3d1d5f0e,0x3dbbff1e,0x3e925ab3,0x3f0d78a0,0x3f800000};
constexpr float mixer_time(std::size_t i){return std::bit_cast<float>(mixer_time_bits[i]);}
class Curve final:public SliderCurve {
 SliderCurveDefinition data_;
public:
 explicit Curve(SliderCurveDefinition data):data_(std::move(data)){}
 float forward(float value)override {
  const auto& p=data_.fields;
  switch(data_.kind){
   case SliderCurveKind::linear:return value;
   case SliderCurveKind::reverse:return 1.f-value;
   case SliderCurveKind::exponential:{
    if(!std::isfinite(value)||value<0.f||value>1.f)throw std::invalid_argument("exponential mapping profile");
    const float result=detail::exponential_exp(value*p[0]);
    return (result-1.f)*p[1];
   }
   case SliderCurveKind::master_volume:{
    if(!std::isfinite(value)||value<0.f||value>1.f)throw std::invalid_argument("master volume mapping profile");
    const float center=p[2];
    if(value<=center)return detail::master_power(value/center,false)*center;
    const float power=detail::master_power(1.f-(value-center)/(1.f-center),false);
    return (1.f-power)*(1.f-center)+center;
   }
   case SliderCurveKind::envelope_time:{
    if(value>=1.f)return 1.f;
    if(value<=0.f)return 0.f;
    const float position=value*20.f;const auto index=static_cast<std::size_t>(position);
    const float fraction=position-static_cast<float>(index);
    return (1.f-fraction)*envelope_time(index)+fraction*envelope_time(index+1);
   }
   case SliderCurveKind::mixer_time:{
    if(value>=1.f)return 1.f;
    if(value<0.f)return 0.f;
    const float position=value*9.f;const auto index=static_cast<std::size_t>(position);
    const float fraction=position-static_cast<float>(index);
    return (1.f-fraction)*mixer_time(index)+fraction*mixer_time(index+1);
   }
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
   case SliderCurveKind::exponential:{
    if(!std::isfinite(value)||value<0.f||value>1.f)throw std::invalid_argument("exponential mapping profile");
    const float argument=value/p[1]+1.f;
    if(!std::isfinite(argument))throw std::invalid_argument("exponential inverse profile");
    return detail::exponential_log(argument)/p[0];
   }
   case SliderCurveKind::master_volume:{
    if(!std::isfinite(value)||value<0.f||value>1.f)throw std::invalid_argument("master volume mapping profile");
    const float center=p[2];
    if(value<=center)return detail::master_power(value/center,true)*center;
    const float power=detail::master_power(1.f-(value-center)/(1.f-center),true);
    return (1.f-power)*(1.f-center)+center;
   }
   case SliderCurveKind::envelope_time:{
    if(value>=1.f)return 1.f;
    if(value<=0.f)return 0.f;
    std::size_t index=0;while(index<19&&value>envelope_time(index+1))++index;
    const float fraction=(value-envelope_time(index))/(envelope_time(index+1)-envelope_time(index));
    return (fraction+static_cast<float>(index))*.05f;
   }
   case SliderCurveKind::mixer_time:{
    if(value>=1.f)return 1.f;
    if(value<0.f)return 0.f;
    std::size_t index=0;while(index<8&&value>mixer_time(index+1))++index;
    const float fraction=(value-mixer_time(index))/(mixer_time(index+1)-mixer_time(index));
    return (fraction+static_cast<float>(index))*std::bit_cast<float>(0x3de38e39u);
   }
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
  case SliderCurveKind::linear:case SliderCurveKind::reverse:case SliderCurveKind::envelope_time:case SliderCurveKind::mixer_time:break;
  case SliderCurveKind::master_volume:if(definition.fields[0]!=2.f||definition.fields[2]!=.5f)return nullptr;break;
  case SliderCurveKind::exponential:if(definition.fields[0]<=0.f||definition.fields[0]>=88.f||definition.fields[1]<=0.f)return nullptr;break;
  case SliderCurveKind::inverse:if(definition.fields[2]==definition.fields[0]||definition.fields[3]==definition.fields[1])return nullptr;break;
  case SliderCurveKind::piecewise:if(definition.fields[0]<=0.f||definition.fields[0]>=1.f)return nullptr;break;
  case SliderCurveKind::harmonic:if(definition.fields[0]<=0.f||definition.fields[0]==1.f)return nullptr;break;
  case SliderCurveKind::table:if(definition.table.size()<3||definition.table.size()>4096)return nullptr;for(std::size_t i=0;i<definition.table.size();++i)if(!std::isfinite(definition.table[i])||(i&&definition.table[i]<=definition.table[i-1]))return nullptr;break;
  default:return nullptr;
 }
 return std::make_unique<Curve>(definition);
}
std::unique_ptr<SliderCurve> make_slider_curve(std::string_view name,std::array<float,4> fields,std::vector<float> table){
 constexpr std::string_view names[]{".?AVLinearPotardMapping@MappingTools@Arturia@@",".?AVReversePotardMapping@MappingTools@Arturia@@",".?AVInversePotardMapping@MappingTools@Arturia@@",".?AVPiecewiseLinearPotardMapping@MappingTools@Arturia@@",".?AVHarmonicMapping@MappingTools@Arturia@@",".?AVTablePotardMapping@MappingTools@Arturia@@",".?AVEnveloppeTimeMapping@MappingTools@Arturia@@",".?AVMixerTimeMapping@MappingTools@Arturia@@"};
 for(unsigned i=0;i<8;++i)if(name==names[i])return make_slider_curve({static_cast<SliderCurveKind>(i),fields,std::move(table)});
 if(name==".?AVBipolarVariableCenterInvPowPotardMapping@MappingTools@Arturia@@")return make_slider_curve({SliderCurveKind::master_volume,fields,{}});
 if(name==".?AVExponentialPotardMapping@MappingTools@Arturia@@")return make_slider_curve({SliderCurveKind::exponential,fields,{}});
 return nullptr;
}
}
