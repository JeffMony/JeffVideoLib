//
// Created by jeffli on 2022/1/13.
//

#include "handler.h"
/**
 *
 * HandlerThread 持有 Looper
 * Handler 持有 Looper
 * Handler 发送消息通过Looper轮转消息
 * Looper 中持有MessageQueue来管理消息
 */

namespace thread {

Handler::Handler(Looper *looper, HandlerCallback *callback)
  : looper_(looper)
  , callback_(callback) {
}

Handler::~Handler() {
}

void Handler::SendMessage(Message *msg) {
  if (looper_) {
    msg->target = this;
    looper_->SendMessage(msg);
  }
}

void Handler::SendMessageAtFront(Message *msg) {
  if (looper_) {
    msg->target = this;
    looper_->SendMessageAtFront(msg);
  }
}

void Handler::DispatchMessage(Message *msg) {
  if (callback_) {
    callback_->HandleMessage(msg);
  }
}

void Handler::DispatchRemoveMessage(Message *msg) {
  if (callback_) {
    callback_->RemoveMessage(msg);
  }
}

void Handler::RemoveMessage(int what) {
  if (looper_) {
    looper_->RemoveMessage(what);
  }
}

int Handler::Size() {
  if (looper_) {
    return looper_->Size();
  }
  return 0;
}

void Handler::DumpMessages() {
  if (looper_) {
    looper_->Dump();
  }
}

}