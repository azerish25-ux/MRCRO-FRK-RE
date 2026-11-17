#pragma once
#include "mfrk/ui_async_updater.hpp"
namespace mfrk::android_ui {
// UiMessageQueue.initialize() must run on the Java main thread first. The
// process dispatcher retains no Activity, scene, updater or callback target.
ui::GuiAsyncUpdateQueue& async_update_queue();
}
