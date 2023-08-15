//
// Created by jeffli on 2022/1/13.
//

#ifndef THREAD_MESSAGE_QUEUE_H_
#define THREAD_MESSAGE_QUEUE_H_

#include "message.h"
#include <pthread.h>
#include <list>

namespace thread {

class Message;

class MessageQueue {
public:
  MessageQueue();

  ~MessageQueue();

  /**
   * 发送的消息放在消息队列结尾
   * @param msg
   */
  void Offer(Message *msg);

  /**
   * 发送的消息放在消息队列开头
   * @param msg
   */
  void OfferAtFront(Message *msg);

  /**
   * 获取消息
   * @return
   */
  Message *Take();

  /**
   * 解锁
   */
  void Notify();

  int Size();

  bool IsEmpty();

  void Clear();

  /**
   * 删除msg.what = what的消息
   * @param what
   */
  void RemoveMessage(int what);

  void Dump();

private:
  pthread_mutex_t queue_mutex_;
  pthread_cond_t queue_cond_;
  /// 存放消息的队列
  std::list<Message *> queue_;
  bool is_destroyed_;

};

}  // namespace thread

#endif  // THREAD_MESSAGE_QUEUE_H_
