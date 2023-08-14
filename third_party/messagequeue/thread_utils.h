//
// Created by jeffli on 2022/1/13.
//

#ifndef THREAD_THREAD_UTILS_H_
#define THREAD_THREAD_UTILS_H_

#include <pthread.h>
#include <string>
#include <functional>

namespace thread {

class ThreadUtils {
public:
  ThreadUtils(std::string name, std::function<void()> runnable);

  ~ThreadUtils();

  void Start();

  bool IsRunning();

  void Interrupt();

  bool IsInterrupted();

  void Stop();

public:
  std::string name_;
  std::function<void()> runnable_;

public:
  static bool Sleep(int64_t time_us);

  static int64_t CurrentThreadId();

private:
  void CreateThread();

private:
  pthread_t thread_;
  pthread_mutex_t mutex_;
  bool is_interrupted_;
};

}  // namespace thread

#endif  // THREAD_THREAD_UTILS_H_
