
#ifndef MEDIA_ENV_H_
#define MEDIA_ENV_H_

#include <jni.h>

int set_java_vm(void *vm);

JavaVM* get_java_vm();

int get_env(JNIEnv** env);

void detach_env();

#endif  // MEDIA_ENV_H_