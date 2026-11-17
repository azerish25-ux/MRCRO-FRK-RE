#include "ui_async_update_queue.hpp"
#include "ui_message_queue.hpp"
namespace mfrk::android_ui {
namespace {
struct AsyncPacket final:ui::GuiUiMessage {
 std::shared_ptr<ui::GuiAsyncUpdateMessage> message;
 explicit AsyncPacket(std::shared_ptr<ui::GuiAsyncUpdateMessage> value):message(std::move(value)){}
};
struct AsyncDispatcher final:ui::GuiMessageListener {
 void received(ui::GuiUiMessage& message)override{
  if(auto* packet=dynamic_cast<AsyncPacket*>(&message))packet->message->deliver();
 }
};
class AndroidAsyncQueue final:public ui::GuiAsyncUpdateQueue {
 std::shared_ptr<ui::GuiMessageListener> dispatcher_=std::make_shared<AsyncDispatcher>();
public:
 bool post(std::shared_ptr<ui::GuiAsyncUpdateMessage> message)override{
  return post_message(dispatcher_,std::make_unique<AsyncPacket>(std::move(message)))!=0;
 }
};
}
ui::GuiAsyncUpdateQueue& async_update_queue(){
 // Matches the process-scoped Java Handler/JNI runtime. No live UI owner is
 // retained here; each packet retains only its independently cancelled word.
 static auto* queue=new AndroidAsyncQueue();return *queue;
}
}
