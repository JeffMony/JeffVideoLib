//
// Created by jeffli on 2022/1/13.
//

#ifndef THREAD_LOOPER_H_
#define THREAD_LOOPER_H_

#include "message.h"
#include "message_queue.h"
#include <cstdlib>
#include <map>
#include <mutex>

namespace thread {

class Message;
class MessageQueue;

class Looper {
public:
  Looper();

  ~Looper();

  /**
   * 创建消息looper实例
   */
  static void Prepare();

  /**
   * 开始执行消息循环
   */
  static void Loop();

  static Looper *MyLooper();

  static int64_t MyLooperId();

  static void Exit();

  /**
   * 如果safely = true, 表示要将消息队列中消息分发完了才行
   *    safely = false, 忽略消息队列中剩余消息, 直接退出
   * @param safely
   */
  void Quit(bool safely);

  void Dump();

  /**
   * 消息队列中消息数
   * @return
   */
  int Size();

  void SendMessage(Message *msg);

  void SendMessageAtFront(Message *msg);

  void RemoveMessage(int what);

private:
  /**
   * 消息循环处理函数
   */
  void LoopInternal();

  void EnqueueMessage(Message *msg);

  void EnqueueMessageAtFront(Message *msg);

  Message *Take();

private:
  bool exiting_;
  bool exited_;
  bool exit_safely_;
  bool looping_;
  pthread_mutex_t variable_mutex_;
  MessageQueue *message_queue_;

};

class LooperManager {
public:
  friend Looper;

  static LooperManager *GetInstance();

public:
  LooperManager();

  ~LooperManager();

  Looper *Create(int64_t tid);

  Looper *Get(int64_t tid);

  void Remove(int64_t tid);

  int Size();

private:
  static LooperManager *instance_;
  std::map<int64_t, Looper *> looper_map_;
  std::mutex looper_mutex_;
};

}  // namespace thread

#endif  // THREAD_LOOPER_H_
