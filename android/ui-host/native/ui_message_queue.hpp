#pragma once
#include "mfrk/ui_message_delivery.hpp"
namespace mfrk::android_ui {
// UiMessageQueue.initialize() must first run on the Java main thread. Posting
// may originate on a native worker; the process queue owns the envelope until
// delivery/cancellation. Handles are monotonic IDs, never native addresses.
std::uint64_t post_message(std::weak_ptr<ui::GuiMessageListener>,std::unique_ptr<ui::GuiUiMessage>);
std::uint64_t post_write(std::weak_ptr<ui::GuiMessageListener>,std::unique_ptr<ui::GuiWriteMessage>);
bool cancel_message(std::uint64_t);
std::uint32_t main_thread_id();
}
