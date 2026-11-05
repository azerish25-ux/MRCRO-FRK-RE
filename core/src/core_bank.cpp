#include "mfrk/core_bank.hpp"

namespace mfrk::core_bank {
bool Side::render(Model m,std::array<float,32>&out) noexcept {
 switch(m){
  case Model::basic:basic_.render(out);for(float&v:out)v=std::bit_cast<float>(0x3ee6e979u)*v;break;
  case Model::super:super_.render_scaled(out);break;
  case Model::harmo:out=harmo_.render_scaled();break;
  case Model::karplus:return karplus_.try_render(out)==mfrk::karplus::Status::ok;
  case Model::va:va_.render(va_controls_.parameters,std::bit_cast<float>(0x3f8ccccdu),out);break;
  case Model::waveshaper:return waveshaper_.try_render_scaled(out)==mfrk::waveshaper::RenderStatus::ok;
  case Model::bass:bass_.render_scaled(out);break;
 }
 return true;
}
bool SlotController::select(Model model) noexcept {
 if(static_cast<unsigned>(model)>static_cast<unsigned>(Model::bass))return false;
 if(side_.active(model))return true;
 side_.active(model,1);
 side_.reset(selected_);
 side_.active(selected_,0);
 selected_=model;
 side_.bind(model,16384);
 side_.phase(model,phase_);
 side_.initialize(model);
 dirty_={1,1,1};
 return true;
}
}
