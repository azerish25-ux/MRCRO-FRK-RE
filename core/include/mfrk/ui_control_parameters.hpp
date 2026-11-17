#pragma once
#include "mfrk/ui_parameter_controller.hpp"
namespace mfrk::ui {
// Source IViewControlled72-byte logical rows. The controller pointers borrow
// actual registered owners; the factory retains their lifetime. Alias flags,
// IDs and controllers use one row list, rather than divergent copied tables.
class GuiControlParameters {
 GuiParameterObserver& owner_;
public:
 std::int32_t preferred_index=-1;
 explicit GuiControlParameters(GuiParameterObserver& owner):owner_(owner){}
 std::uint32_t id(std::uint64_t index)const noexcept{
  return index<owner_.aliases.size()?owner_.aliases[static_cast<std::size_t>(index)].id:0xffffffffu;
 }
 GuiParameterController& at(std::uint64_t index){
  if(index>=owner_.aliases.size())throw std::out_of_range("Parameter controller index");
  auto* controller=owner_.aliases[static_cast<std::size_t>(index)].controller;
  if(!controller)throw std::logic_error("Registered parameter controller required");
  return *controller;
 }
 GuiParameterController& find(std::uint32_t parameter){
  if(preferred_index!=-1)parameter=id(static_cast<std::uint64_t>(preferred_index));
  auto at=owner_.aliases.begin();
  if(parameter!=0xffffffffu)while(at!=owner_.aliases.end()&&at->id!=parameter)++at;
  if(at==owner_.aliases.end()||!at->controller)throw std::out_of_range("Parameter controller ID");
  return *at->controller;
 }
};
}
