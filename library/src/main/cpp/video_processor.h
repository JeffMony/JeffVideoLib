//
// Created by jefflee on 2023/8/15.
//

#ifndef JEFFVIDEOLIB_LIBRARY_SRC_MAIN_CPP_VIDEO_PROCESSOR_H_
#define JEFFVIDEOLIB_LIBRARY_SRC_MAIN_CPP_VIDEO_PROCESSOR_H_

#include <jni.h>
#include <map>
#include "handler.h"
#include "handler_thread.h"

namespace media {

 class VideoProcessor : public thread::HandlerCallback {
 public:
  explicit VideoProcessor(jobject object);

  virtual ~VideoProcessor();

  void TransformVideo(const char *input_path, const char *output_path, jobject listener);

 protected:
   void HandleMessage(thread::Message *msg) override;

   void RemoveMessage(thread::Message *msg) override;

 private:
   void InitInternal();

   void TransformVideoInternal(const char *input_path, const char *output_path, jobject listener);

 private:
   void RemoveListener(int key);

   jobject GetListener(int key);

   void UpdateListener(int key, jobject listener);

 private:
   void CallOnTransformFailed(jobject listener, int err);

   void CallOnTransformProgress(jobject listener, float progress);

   void CallOnTransformFinished(jobject listener);

 private:
  jobject video_process_object_;
  thread::HandlerThread *handler_thread_;
  thread::Handler *handler_;
  std::map<int, jobject> listener_map_;
  int listener_key_;
  pthread_mutex_t listener_mutex_;
  float transform_progress_;

};

} // media

#endif //JEFFVIDEOLIB_LIBRARY_SRC_MAIN_CPP_VIDEO_PROCESSOR_H_
