//
// Created by jeff lee on 2023/4/5.
//

#ifndef MESSAGEQUEUE_LIBRARY_SRC_MAIN_CPP_STRING_WRAPPER_H_
#define MESSAGEQUEUE_LIBRARY_SRC_MAIN_CPP_STRING_WRAPPER_H_

namespace thread {

struct StringWrapper {
 public:
  explicit StringWrapper()
  : v1(nullptr)
  , v2(nullptr) {

  }

  explicit StringWrapper(const char *v1) {
    this->v1 = v1;
  }

  explicit StringWrapper(const char *v1, const char *v2) {
    this->v1 = v1;
    this->v2 = v2;
  }

  virtual ~StringWrapper() {
    if (v1 != nullptr) {
      delete[] v1;
      v1 = nullptr;
    }
    if (v2 != nullptr) {
      delete[] v2;
      v2 = nullptr;
    }
  }

  const char *getV1() {
    return v1;
  }

  const char *getV2() {
    return v2;
  }

 private:
  const char *v1;
  const char *v2;

};

}

#endif //MESSAGEQUEUE_LIBRARY_SRC_MAIN_CPP_STRING_WRAPPER_H_
