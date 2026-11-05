#include "mfrk/gate_dispatch.hpp"
namespace mfrk::gate_dispatch {
bool dispatch(State& state,std::uint8_t row,std::uint8_t status,bool para,std::uint8_t cyclic_mode,bool disabled,Plan& plan) noexcept {
 if(row>=12||(status!=1&&status!=2)||cyclic_mode>2)return false;
 if(status==2&&cyclic_mode==2&&!disabled&&state.active_count<2&&(state.first_row>=12||state.first_sides[state.first_row%6]>1))return false;
 const std::uint8_t voice=row%6,side=row/6;const bool on=status==1;
 if(on){state.active_count=static_cast<std::uint8_t>(state.active_count+1);state.last_row=row;state.last_sides[voice]=side;if(state.active_count<2){state.first_row=row;state.first_sides[voice]=side;}}
 plan.count=0;auto add=[&](Kind kind,std::uint8_t v,std::uint8_t s,bool gate){plan.actions[plan.count++]={kind,v,s,gate};};
 if(para)add(Kind::oscillator,voice,side,on);else{add(Kind::oscillator,voice,0,on);add(Kind::oscillator,voice,1,on);}
 add(Kind::main,voice,side,on);
 if(!disabled){
  if(cyclic_mode==0)add(Kind::cyclic,voice,side,on);
  else if(cyclic_mode==1)add(Kind::cyclic,0,0,on);
  else if(state.active_count<2){if(on)add(Kind::cyclic,voice,side,true);else{const auto first=static_cast<std::uint8_t>(state.first_row%6);add(Kind::cyclic,first,state.first_sides[first],false);}}
 }
 if(!on&&state.active_count)state.active_count=static_cast<std::uint8_t>(state.active_count-1);
 return true;
}
}
