//
// Created by jeffli on 2022/1/13.
//

#include "looper.h"
#include "thread_utils.h"
#include "android_log.h"
#include <cassert>
#include "time_utils.h"

namespace thread {

Looper::Looper()
  : exiting_(false)
  , exited_(false)
  , exit_safely_(false)
  , looping_(false) {
  message_queue_ = new MessageQueue();
  pthread_mutex_init(&variable_mutex_, nullptr);
}

Looper::~Looper() {
  LOGI("enter: %s %s %d", __FILE_NAME__, __func__ , __LINE__);
  pthread_mutex_lock(&variable_mutex_);
  if (message_queue_) {
    delete message_queue_;
    message_queue_ = nullptr;
  }
  exiting_ = false;
  exited_ = true;
  looping_ = false;
  pthread_mutex_unlock(&variable_mutex_);

  pthread_mutex_destroy(&variable_mutex_);
  LOGI("leave: %s %s %d", __FILE_NAME__, __func__ , __LINE__);
}

void Looper::Prepare() {
  int64_t tid = ThreadUtils::CurrentThreadId();
  Looper *looper = LooperManager::GetInstance()->Create(tid);
  if (looper == nullptr) {
    LOGE("%s %s %d Current thread looper has been called", __FILE_NAME__, __func__ , __LINE__);
  }
}

void Looper::Loop() {
  MyLooper()->LoopInternal();
}

Looper * Looper::MyLooper() {
  int64_t tid = ThreadUtils::CurrentThreadId();
  Looper *looper = LooperManager::GetInstance()->Get(tid);
  if (looper == nullptr) {
    LOGE("%s %s %d Please invoke DuLooper::Prepare first", __FILE_NAME__, __func__ , __LINE__);
  }
  assert(looper);
  return looper;
}

int64_t Looper::MyLooperId() {
  return reinterpret_cast<int64_t>(MyLooper());
}

void Looper::Exit() {
  int64_t tid = ThreadUtils::CurrentThreadId();
  LooperManager::GetInstance()->Remove(tid);
}

void Looper::Quit(bool safely) {
  if (exited_) {
    return;
  }
  pthread_mutex_lock(&variable_mutex_);
  if (exiting_ || exited_) {
    pthread_mutex_unlock(&variable_mutex_);
    return;
  }
  exit_safely_ = safely;
  exiting_ = true;
  pthread_mutex_unlock(&variable_mutex_);
  LOGI("%s %s %d Message queue size=%d", __FILE_NAME__, __func__ , __LINE__, message_queue_->Size());
  Dump();
  if (message_queue_ != nullptr){
    message_queue_->Notify();
  }
}

void Looper::Dump() {
  if (message_queue_ != nullptr) {
    message_queue_->Dump();
  }
}

int Looper::Size() {
  if (message_queue_ != nullptr) {
    return message_queue_->Size();
  }
  return 0;
}

void Looper::SendMessage(Message *msg) {
  if (exited_) {
    return;
  }
  pthread_mutex_lock(&variable_mutex_);
  if (exiting_ || exited_) {
    pthread_mutex_unlock(&variable_mutex_);
    return;
  }
  pthread_mutex_unlock(&variable_mutex_);
  EnqueueMessage(msg);
}

void Looper::SendMessageAtFront(Message *msg) {
  if (exited_) {
    return;
  }
  pthread_mutex_lock(&variable_mutex_);
  if (exiting_ || exited_) {
    pthread_mutex_unlock(&variable_mutex_);
    return;
  }
  pthread_mutex_unlock(&variable_mutex_);
  EnqueueMessageAtFront(msg);
}

void Looper::RemoveMessage(int what) {
  if (message_queue_ != nullptr) {
    message_queue_->RemoveMessage(what);
  }
}

void Looper::LoopInternal() {
  if (exited_) {
    return;
  }
  pthread_mutex_lock(&variable_mutex_);
  if (looping_ || exiting_ || exited_) {
    pthread_mutex_unlock(&variable_mutex_);
    return;
  }
  looping_ = true;
  pthread_mutex_unlock(&variable_mutex_);

  for (;;) {
    Message *msg = Take();
    if (msg) {
      if (msg->target) {
        msg->target->DispatchMessage(msg);
      }
      delete msg;
    }
    if (exited_) {
      break;
    }
    pthread_mutex_lock(&variable_mutex_);
    if (exit_safely_) {
      if (exiting_ && message_queue_->Size() == 0) {
        pthread_mutex_unlock(&variable_mutex_);
        break;
      }
    } else {
      if (exiting_) {
        pthread_mutex_unlock(&variable_mutex_);
        break;
      }
    }
    pthread_mutex_unlock(&variable_mutex_);
  }

  LOGI("%s %s %d Exit Message Loop", __FILE_NAME__, __func__ , __LINE__);
  if (message_queue_ == nullptr || exited_) {
    return;
  }

  int64_t time = TimeUtils::GetCurrentTimeUs();
  while (message_queue_->Size() > 0) {
    Message *msg = message_queue_->Take();
    if (msg) {
      delete msg;
    }
  }
  message_queue_->Clear();
  LOGI("%s %s %d Clear message_queue cost time=%lld us", __FILE_NAME__, __func__ , __LINE__, (TimeUtils::GetCurrentTimeUs() - time));

  pthread_mutex_lock(&variable_mutex_);
  exiting_ = false;
  exited_ = true;
  looping_ = false;
  pthread_mutex_unlock(&variable_mutex_);
}

void Looper::EnqueueMessage(Message *msg) {
  /// TODO msg 模式, 可以放在队头, 也可以放在队尾
  if (message_queue_ != nullptr) {
    message_queue_->Offer(msg);
  }
}

void Looper::EnqueueMessageAtFront(Message *msg) {
  if (message_queue_ != nullptr) {
    message_queue_->OfferAtFront(msg);
  }
}

Message * Looper::Take() {
  if (message_queue_ != nullptr) {
    return message_queue_->Take();
  }
  return nullptr;
}

/// ------------------------------------------------------------------

LooperManager *LooperManager::instance_ = new LooperManager();

LooperManager::LooperManager() {

}

LooperManager::~LooperManager() {

}

LooperManager * LooperManager::GetInstance() {
  return instance_;
}

Looper * LooperManager::Create(int64_t tid) {
  std::lock_guard<std::mutex> guard(looper_mutex_);
  auto it = looper_map_.find(tid);
  if (it == looper_map_.end()) {
    Looper *looper = new Looper();
    looper_map_[tid] = looper;
    return looper;
  }
  return nullptr;
}

Looper * LooperManager::Get(int64_t tid) {
  std::lock_guard<std::mutex> guard(looper_mutex_);
  auto it = looper_map_.find(tid);
  if (it == looper_map_.end()) {
    return nullptr;
  }
  return it->second;
}

void LooperManager::Remove(int64_t tid) {
  std::lock_guard<std::mutex> guard(looper_mutex_);
  auto it = looper_map_.find(tid);
  if (it != looper_map_.end()) {
    looper_map_.erase(it);
  }
}

int LooperManager::Size() {
  std::lock_guard<std::mutex> guard(looper_mutex_);
  return looper_map_.size();
}

}