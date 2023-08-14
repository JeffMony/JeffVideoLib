//
// Created by jeffli on 2022/1/13.
//

#include "thread_utils.h"
#include "android_log.h"
#include <thread>

namespace thread {

ThreadUtils::ThreadUtils(std::string name, std::function<void()> runnable)
  : name_(name)
  , runnable_(runnable)
  , is_interrupted_(false) {
  pthread_mutex_init(&mutex_, nullptr);
}

ThreadUtils::~ThreadUtils() {
  LOGI("%s %s %d DuThread destroy", __FILE_NAME__, __func__ , __LINE__);
  runnable_ = nullptr;
  Stop();
}

static void *RunTask(void *args) {
  ThreadUtils *thread = static_cast<ThreadUtils *>(args);
  LOGI("%s %s %d tid=%lld start", __FILE_NAME__, __func__ , __LINE__, pthread_self());
  thread->runnable_();
  LOGI("%s %s %d tid=%lld stop, name=%s", __FILE_NAME__, __func__ , __LINE__, pthread_self(), thread->name_.c_str());
  pthread_exit(nullptr);
}

void ThreadUtils::Start() {
  is_interrupted_ = false;
  CreateThread();
}

bool ThreadUtils::IsRunning() {
  return !IsInterrupted();
}

void ThreadUtils::Interrupt() {
  pthread_mutex_lock(&mutex_);
  if (is_interrupted_) {
    pthread_mutex_unlock(&mutex_);
    return;
  }
  is_interrupted_ = true;
  pthread_mutex_unlock(&mutex_);
  if (pthread_join(thread_, nullptr)) {
    LOGE("%s %s %d tid=%lld join failed", __FILE_NAME__, __func__ , __LINE__, thread_);
  }
}

bool ThreadUtils::IsInterrupted() {
  pthread_mutex_lock(&mutex_);
  bool ret = is_interrupted_;
  pthread_mutex_unlock(&mutex_);
  return ret;
}

void ThreadUtils::Stop() {
  pthread_join(thread_, nullptr);
  pthread_mutex_destroy(&mutex_);
}

bool ThreadUtils::Sleep(int64_t time_us) {
  std::this_thread::sleep_for(std::chrono::nanoseconds(time_us * 1000));
  return true;
}

int64_t ThreadUtils::CurrentThreadId() {
  return pthread_self();
}

void ThreadUtils::CreateThread() {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&thread_, &attr, RunTask, (void *) this);
}

}
