//
// Created by jeffli on 2022/1/13.
//

#ifndef THREAD_MESSAGE_H_
#define THREAD_MESSAGE_H_

#include "handler.h"
#include <string>

namespace thread {

class Handler;

class Message {
public:
  Message();

  ~Message();

public:
  int what;
  int arg1;
  int arg2;
  int arg3;
  int arg4;
  int arg5;
  int arg6;
  int arg7;
  bool flag1;
  void *obj1;
  void *obj2;
  void *obj3;
  void *obj4;
  std::string str1;
  std::string str2;

public:
  Handler *target;
};

}  // namespace thread

#endif  // THREAD_MESSAGE_H_
