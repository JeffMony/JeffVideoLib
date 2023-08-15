
#include "media_env.h"
#include <pthread.h>
#include "android_log.h"

static JavaVM* java_vm;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int set_java_vm(void *vm) {
  int ret = 0;
  pthread_mutex_lock(&lock);
  if (java_vm == NULL) {
    java_vm = vm;
  } else if (java_vm != vm) {
    ret = -1;
  }
  pthread_mutex_unlock(&lock);
  return ret;
}

JavaVM* get_java_vm() {
  void *vm;
  pthread_mutex_lock(&lock);
  vm = java_vm;
  pthread_mutex_unlock(&lock);
  return vm;
}

int get_env(JNIEnv** env) {
  JavaVM *vm = get_java_vm();
  int ret = (*vm)->GetEnv(vm, (void **) env, JNI_VERSION_1_6);
  if (ret == JNI_EDETACHED) {
    if ((*vm)->AttachCurrentThread(vm, env, NULL) != JNI_OK) {
      LOGE("%s %s LINE=%d Failed to attach the JNI environment to the current thread", __FILE_NAME__, __func__, __LINE__);
      *env = NULL;
      return -10;
    }
  }
  return ret;
}

void detach_env() {
  JavaVM *vm = get_java_vm();
  (*vm)->DetachCurrentThread(vm);
}