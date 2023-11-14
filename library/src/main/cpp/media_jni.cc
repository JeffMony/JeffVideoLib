//
// Created by jefflee on 2023/8/15.
//

#include <jni.h>
#include "video_processor.h"

extern "C" {
#include "media_env.h"
#include "android_log.h"
}

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

#define VIDEO_PROCESS "com/jeffmony/m3u8library/VideoProcessManager"

jlong VIDEO_PROCESS_CREATE_HANDLER(JNIEnv *env, jobject object) {
  jobject video_process_object = env->NewGlobalRef(object);
  auto video_processor = new media::VideoProcessor(video_process_object);
  return reinterpret_cast<jlong>(video_processor);
}

void VIDEO_PROCESS_TRANSFORM_VIDEO(JNIEnv *env, jobject object, jlong id, jstring j_input_path, jstring j_output_path, jobject j_listener) {
  auto video_processor = reinterpret_cast<media::VideoProcessor *>(id);
  auto input_path = env->GetStringUTFChars(j_input_path, JNI_FALSE);
  auto output_path = env->GetStringUTFChars(j_output_path, JNI_FALSE);
  jobject listener = env->NewGlobalRef(j_listener);
  video_processor->TransformVideo(input_path, output_path, listener);
  env->ReleaseStringUTFChars(j_input_path, input_path);
  env->ReleaseStringUTFChars(j_output_path, output_path);
}

void VIDEO_PROCESS_TRANSFORM_VIDEO_CONFIG(JNIEnv *env, jobject object, jlong id, jstring j_config, jobject j_listener) {
  auto video_processor = reinterpret_cast<media::VideoProcessor *>(id);
  auto config = env->GetStringUTFChars(j_config, JNI_FALSE);
  jobject listener = env->NewGlobalRef(j_listener);
  video_processor->TransformVideo(config, listener);
  env->ReleaseStringUTFChars(j_config, config);
}

static JNINativeMethod videoProcessMethods[] = {
    { "createHandler", "()J", (void **) VIDEO_PROCESS_CREATE_HANDLER },
    { "transformM3U8ToMp4", "(JLjava/lang/String;Ljava/lang/String;Lcom/jeffmony/m3u8library/listener/IVideoTransformListener;)V", (void **) VIDEO_PROCESS_TRANSFORM_VIDEO },
    {"transformToMp4", "(JLjava/lang/String;Lcom/jeffmony/m3u8library/listener/IVideoTransformListener;)V", (void **) VIDEO_PROCESS_TRANSFORM_VIDEO_CONFIG },
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  JNIEnv *env = nullptr;
  if ((vm)->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }
  set_java_vm(vm);
  auto video_process = env->FindClass(VIDEO_PROCESS);
  jint video_process_result = env->RegisterNatives(video_process, videoProcessMethods, NELEM(videoProcessMethods));
  LOGE("%s %s %d video_process_result=%d", __FILE_NAME__, __func__ , __LINE__, (video_process_result == JNI_OK));
  env->DeleteLocalRef(video_process);
  av_log_set_level(AV_LOG_INFO);
  av_log_set_callback(ffp_log_callback_report);
  return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnLoad(JavaVM *vm, void *reserved) {
  JNIEnv *env = nullptr;
  if ((vm)->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
    return;
  }
}

#ifdef __cplusplus
}
#endif