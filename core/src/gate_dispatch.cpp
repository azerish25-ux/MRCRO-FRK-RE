#include "mfrk/gate_dispatch.hpp"
namespace mfrk::gate_dispatch {
bool dispatch(State& state,std::uint8_t row,std::uint8_t status,bool para,std::uint8_t cyclic_mode,bool disabled,Plan& plan,const ResetContext* reset_context) noexcept {
 if(row>=12||status<1||status>5||cyclic_mode>2||(status==5&&para&&!reset_context))return false;
 if(status==2&&cyclic_mode==2&&!disabled&&state.active_count<2&&(state.first_row>=12||state.first_sides[state.first_row%6]>1))return false;
 const std::uint8_t voice=row%6,side=row/6;const bool on=status!=2&&status!=5;
 if(on){if(status==1)state.active_count=static_cast<std::uint8_t>(state.active_count+1);state.last_row=row;state.last_sides[voice]=side;if(state.active_count<2){state.first_row=row;state.first_sides[voice]=side;}}
 plan.count=0;auto add=[&](Kind kind,std::uint8_t v,std::uint8_t s,bool gate){plan.actions[plan.count++]={kind,v,s,gate};};
 if(para)add(Kind::oscillator,voice,side,on);else{add(Kind::oscillator,voice,0,on);add(Kind::oscillator,voice,1,on);}
 if(status==5){if(para&&(!reset_context->other_active||!reset_context->other_enabled))add(Kind::voice_reset,voice,side,false);add(Kind::main_reset,voice,side,false);add(Kind::cyclic_reset,voice,side,false);add(Kind::lfo1_reset,voice,side,false);add(Kind::lfo2_reset,voice,side,false);if(state.active_count)state.active_count=static_cast<std::uint8_t>(state.active_count-1);return true;}
 if(status!=3)add(Kind::main,voice,side,on);
 if(!disabled){
  if(cyclic_mode==0)add(Kind::cyclic,voice,side,on);
  else if(cyclic_mode==1)add(Kind::cyclic,0,0,on);
  else if(state.active_count<2){if(status==1)add(Kind::cyclic,voice,side,true);else if(!on){const auto first=static_cast<std::uint8_t>(state.first_row%6);add(Kind::cyclic,first,state.first_sides[first],false);}}
 }
 if(!on&&state.active_count)state.active_count=static_cast<std::uint8_t>(state.active_count-1);
 return true;
}
bool plan_rows(State& state,const std::array<notepath::NoteRow,12>& rows,bool para,std::uint8_t cyclic_mode,bool disabled,RowsPlan& result,const std::uint16_t* row_mask) noexcept {
 if(cyclic_mode>2)return false;
 auto next=state;RowsPlan output;
 for(unsigned voice=0;voice<6;++voice)for(unsigned side=0;side<2;++side){
  const auto row=static_cast<std::uint8_t>(voice+side*6);const auto status=rows[row].bytes[7];if(!status)continue;
  const unsigned other=voice+(side==0?6:0);ResetContext reset{rows[other].bytes[0]!=0,row_mask&&((*row_mask&(1u<<other))!=0)};
  Plan plan;if(!dispatch(next,row,status,para,cyclic_mode,disabled,plan,(status==5&&para&&row_mask)?&reset:nullptr))return false;
  for(unsigned i=0;i<plan.count;++i)output.actions[output.count++]=plan.actions[i];
 }
 state=next;result=output;return true;
}
}
