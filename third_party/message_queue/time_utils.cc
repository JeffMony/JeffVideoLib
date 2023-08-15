//
// Created by jeffli on 2022/1/13.
//

#include "time_utils.h"

int64_t thread::TimeUtils::GetCurrentTimeUs() {
  struct timeval time;
  gettimeofday(&time, nullptr);
  return static_cast<int64_t>(time.tv_sec * 1000000.0 + time.tv_usec);
}
