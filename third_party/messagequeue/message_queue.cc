//
// Created by jeffli on 2022/1/13.
//

#include "message_queue.h"
#include "android_log.h"
#include <sstream>

namespace thread {

MessageQueue::MessageQueue()
  : is_destroyed_(false) {
  pthread_mutex_init(&queue_mutex_, nullptr);
  pthread_cond_init(&queue_cond_, nullptr);
}

MessageQueue::~MessageQueue() {
  LOGI("enter: %s %s %d", __FILE_NAME__, __func__ , __LINE__);
  pthread_mutex_lock(&queue_mutex_);
  is_destroyed_ = true;
  pthread_mutex_unlock(&queue_mutex_);
  pthread_mutex_destroy(&queue_mutex_);
  pthread_cond_destroy(&queue_cond_);
  LOGI("leave: %s %s %d", __FILE_NAME__, __func__ , __LINE__);
}

void MessageQueue::Offer(Message *msg) {
  if (is_destroyed_) {
    return;
  }
  pthread_mutex_lock(&queue_mutex_);
  if (is_destroyed_) {
    pthread_mutex_unlock(&queue_mutex_);
    return;
  }
  queue_.push_back(msg);
  pthread_cond_broadcast(&queue_cond_);
  pthread_mutex_unlock(&queue_mutex_);
}

void MessageQueue::OfferAtFront(Message *msg) {
  if (is_destroyed_) {
    return;
  }
  pthread_mutex_lock(&queue_mutex_);
  if (is_destroyed_) {
    pthread_mutex_unlock(&queue_mutex_);
    return;
  }
  queue_.push_front(msg);
  pthread_cond_broadcast(&queue_cond_);
  pthread_mutex_unlock(&queue_mutex_);
}

Message *MessageQueue::Take() {
  if (is_destroyed_) {
    return nullptr;
  }
  pthread_mutex_lock(&queue_mutex_);
  if (Size() <= 0) {
//    LOGV("%s %s %d Message queue is empty, should wait", __FILE_NAME__, __func__ , __LINE__);
    pthread_cond_wait(&queue_cond_, &queue_mutex_);
//    LOGV("%s %s %d Message queue continues to work", __FILE_NAME__, __func__ , __LINE__);
  }
  if (queue_.empty()) {
    pthread_mutex_unlock(&queue_mutex_);
    return nullptr;
  }
  Message *msg = queue_.front();
  queue_.pop_front();
  pthread_mutex_unlock(&queue_mutex_);
  return msg;
}

void MessageQueue::Notify() {
  if (is_destroyed_) {
    return;
  }
  pthread_mutex_lock(&queue_mutex_);
  pthread_cond_broadcast(&queue_cond_);
  pthread_mutex_unlock(&queue_mutex_);
}

int MessageQueue::Size() {
  return queue_.size();
}

bool MessageQueue::IsEmpty() {
  return queue_.empty();
}

void MessageQueue::Clear() {
  if (is_destroyed_) {
    return;
  }
  Notify();
  if (queue_.empty()) {
    return;
  }
  pthread_mutex_lock(&queue_mutex_);
  while (!queue_.empty()) {
    Message *msg = queue_.front();
    queue_.pop_front();
    if (msg) {
      delete msg;
    }
  }
  queue_.clear();
  pthread_mutex_unlock(&queue_mutex_);
}

void MessageQueue::RemoveMessage(int what) {
  if (is_destroyed_) {
    return;
  }
  pthread_mutex_lock(&queue_mutex_);
  std::list<Message *>::iterator it = queue_.begin();
  while (it != queue_.end()) {
    Message *msg = *it;
    if (what == msg->what) {
      msg->target->DispatchRemoveMessage(msg);
      delete msg;
      it = queue_.erase(it);
      continue;
    }
    ++it;
  }
  pthread_mutex_unlock(&queue_mutex_);
}

void MessageQueue::Dump() {
  if (is_destroyed_) {
    return;
  }
  pthread_mutex_lock(&queue_mutex_);
  std::ostringstream os;
  std::list<Message *>::iterator it = queue_.begin();
  while (it != queue_.end()) {
    Message *msg = *it;
    os << msg->what<<"\n";
    ++it;
  }
  LOGI("%s %s %d MessageQueue Result=%s", __FILE_NAME__, __func__ , __LINE__, os.str().c_str());
  pthread_mutex_unlock(&queue_mutex_);
}

}