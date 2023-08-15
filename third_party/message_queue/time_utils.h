//
// Created by jeffli on 2022/1/13.
//

#ifndef THREAD_TIME_UTILS_H_
#define THREAD_TIME_UTILS_H_

#include <sys/time.h>

namespace thread {

class TimeUtils {
public:

  static int64_t GetCurrentTimeUs();

};

}  // namespace thread

#endif  // THREAD_TIME_UTILS_H_
