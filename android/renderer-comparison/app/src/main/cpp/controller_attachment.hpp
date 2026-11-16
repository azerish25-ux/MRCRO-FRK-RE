#pragma once
#include "mfrk/ui_slider_controller.hpp"
#include <cstdint>
#include <memory>
namespace mfrk::androidui {
// The owner must retain every host/config/state referenced by its controller.
// Attach and dispatch on the Android UI thread; handles are opaque IDs, never
// native addresses. Detachment finishes any active edit before releasing owner.
struct ControllerAttachment {
 virtual ~ControllerAttachment()=default;
 virtual ui::SliderController& controller()=0;
};
std::uint64_t attach_controller(std::shared_ptr<ControllerAttachment>);
void detach_controller(std::uint64_t);
std::shared_ptr<ControllerAttachment> controller_attachment(std::uint64_t);
}
