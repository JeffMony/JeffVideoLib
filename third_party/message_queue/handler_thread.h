//
// Created by jeffli on 2022/1/13.
//

#ifndef THREAD_HANDLER_THREAD_H_
#define THREAD_HANDLER_THREAD_H_

#include <pthread.h>
#include <string>
#include "handler.h"
#include "looper.h"

namespace thread {

class HandlerThread {
public:
  static HandlerThread *Create(std::string name);

  void RunInternal();

private:
  HandlerThread(std::string name);

public:
  ~HandlerThread();

  void Quit();

  bool QuitSafely();

  Looper *GetLooper();

private:
  std::string name_;
  pthread_t thread_;
  pthread_mutex_t mutex_;
  pthread_cond_t cond_;
  Looper *looper_;
  bool exiting_;
  bool exited_;
};

}  // namespace thread

#endif  // THREAD_HANDLER_THREAD_H_
