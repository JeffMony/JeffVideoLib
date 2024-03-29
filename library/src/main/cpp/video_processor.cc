//
// Created by jefflee on 2023/8/15.
//

#include "video_processor.h"
#include "video_processor_message.h"
#include "ffmpeg_options_const.h"

extern "C" {
#include "media_env.h"
#include "android_log.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
#include "libavutil/timestamp.h"
}

namespace media {

VideoProcessor::VideoProcessor(jobject object)
  : video_process_object_(object)
  , handler_(nullptr)
  , handler_thread_(nullptr)
  , transform_progress_(0.0f) {
  std::string name("Video-Processor-Thread");
  handler_thread_ = thread::HandlerThread::Create(name);
  handler_ = new thread::Handler(handler_thread_->GetLooper(), this);
}

VideoProcessor::~VideoProcessor() {

}

void VideoProcessor::TransformVideo(const char *input_path, const char *output_path, jobject listener) {
  auto length = strlen(input_path) + 1;
  auto input_str = new char[length];
  snprintf(input_str, length, "%s%c", input_path, 0);
  length = strlen(output_path) + 1;
  auto output_str = new char[length];
  snprintf(output_str, length, "%s%c", output_path, 0);
  thread::Message *msg = new thread::Message();
  msg->what = kTransformVideo;
  msg->obj1 = input_str;
  msg->obj2 = output_str;
  msg->obj3 = listener;
  handler_->SendMessage(msg);
}

void VideoProcessor::TransformVideo(const char *config, jobject listener) {
  auto length = strlen(config) + 1;
  auto input_config = new char[length];
  snprintf(input_config, length, "%s%c", config, 0);
  thread::Message *msg = new thread::Message();
  msg->what = kTransformVideoConfig;
  msg->obj1 = input_config;
  msg->obj2 = listener;
  handler_->SendMessage(msg);
}

void VideoProcessor::TransformVideoInternal(const char *input_path, const char *output_path, jobject listener) {
  /// 初始化的时候设置options,可以解决av_dict_set导致的NE问题
  AVDictionary *ffmpeg_options = NULL;
  av_dict_set(&ffmpeg_options, PROTOCOL_OPTION_KEY, PROTOCOL_OPTION_VALUE, 0);
  av_dict_set(&ffmpeg_options, FORMAT_EXTENSION_KEY, FORMAT_EXTENSION_VALUE, 0);
  av_dict_set(&ffmpeg_options, "fflags", "discardcorrupt", 0);

  AVFormatContext *input_context = NULL;
  int ret = avformat_open_input(&input_context, input_path, 0, &ffmpeg_options);
  if (ret < 0) {
    av_dict_free(&ffmpeg_options);
    avformat_close_input(&input_context);
    LOGE("%s %s %d avformat_open_input failed msg=%s", __FILE_NAME__, __func__ , __LINE__, av_err2str(ret));
    CallOnTransformFailed(listener, -2);
    return;
  }
  ret = avformat_find_stream_info(input_context, NULL);
  if (ret < 0) {
    av_dict_free(&ffmpeg_options);
    avformat_close_input(&input_context);
    LOGE("%s %s %d avformat_find_stream_info failed msg=%s", __FILE_NAME__, __func__ , __LINE__, av_err2str(ret));
    CallOnTransformFailed(listener, -3);
    return;
  }
  av_dump_format(input_context, 1, input_path, 0);

  AVFormatContext *output_context = NULL;
  ret = avformat_alloc_output_context2(&output_context, NULL, NULL, output_path);
  if (ret < 0) {
    av_dict_free(&ffmpeg_options);
    avformat_close_input(&input_context);
    avformat_free_context(output_context);
    LOGE("%s %s %d avformat_alloc_output_context2 failed msg=%s", __FILE_NAME__, __func__ , __LINE__, av_err2str(ret));
    CallOnTransformFailed(listener, -4);
    return;
  }
  LOGI("%s %s %d output format=%s", __FILE_NAME__, __func__ , __LINE__, output_context->oformat->name);
  int stream_size = input_context->nb_streams;
  int *streams = (int *) av_malloc_array(stream_size, sizeof(*streams));
  int width;
  int height;
  int stream_index = 0;
  int64_t video_duration = 0;
  int video_stream_index = -1;
  for (int index = 0; index < stream_size; index++) {
    AVStream *out_stream;
    AVStream *in_stream = input_context->streams[index];
    AVCodecParameters *in_codecpar = in_stream->codecpar;

    if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
    in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
    in_codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
      streams[index] = -1;
      continue;
    }

    if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index = index;
      width = in_codecpar->width;
      height = in_codecpar->height;
      if (width == 0 || height == 0) {
        avformat_close_input(&input_context);
        avformat_free_context(output_context);
        av_dict_free(&ffmpeg_options);
        av_freep(&streams);
        LOGE("%s %s %d input file has no width or height, w=%d h=%d", __FILE_NAME__, __func__ , __LINE__, width, height);
        CallOnTransformFailed(listener, -5);
        return;
      }
      video_duration = av_rescale_q(in_stream->duration, in_stream->time_base, AV_TIME_BASE_Q) / 1000;
      LOGI("%s %s %d video_duration=%d", __FILE_NAME__, __func__ , __LINE__, video_duration);
    }

    streams[index] = stream_index++;
    out_stream = avformat_new_stream(output_context, NULL);
    if (!out_stream) {
      avformat_close_input(&input_context);
      avformat_free_context(output_context);
      av_dict_free(&ffmpeg_options);
      av_freep(&streams);
      LOGE("%s %s %d Failed to allocate output stream", __FILE_NAME__, __func__ , __LINE__);
      CallOnTransformFailed(listener, -6);
      return;
    }
    ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
    if (ret < 0) {
      avformat_close_input(&input_context);
      avformat_free_context(output_context);
      av_dict_free(&ffmpeg_options);
      av_freep(&streams);
      LOGE("%s %s %d avcodec_parameters_copy failed msg=%s", __FILE_NAME__, __func__ , __LINE__, av_err2str(ret));
      CallOnTransformFailed(listener, -7);
      return;
    }
    out_stream->codecpar->codec_tag = 0;
  }
  if (video_duration < 0) {
    video_duration = input_context->duration;
    video_duration = video_duration / 1000;
  }
  if (video_stream_index == -1) {
    avformat_close_input(&input_context);
    avformat_free_context(output_context);
    av_dict_free(&ffmpeg_options);
    av_freep(&streams);
    LOGE("%s %s %d Input file has no video stream", __FILE_NAME__, __func__ , __LINE__);
    CallOnTransformFailed(listener, -8);
    return;
  }
  av_dump_format(output_context, 0, output_path, 1);
  if (!(output_context->flags & AVFMT_NOFILE)) {
    ret = avio_open(&output_context->pb, output_path, AVIO_FLAG_WRITE);
    if (ret < 0) {
      avformat_close_input(&input_context);
      if (output_context && output_context->pb) {
        avio_closep(&output_context->pb);
      }
      avformat_free_context(output_context);
      av_dict_free(&ffmpeg_options);
      av_freep(&streams);
      CallOnTransformFailed(listener, -9);
      return;
    }
  }
  ret = avformat_write_header(output_context, &ffmpeg_options);
  if (ret < 0) {
    avformat_close_input(&input_context);
    if (output_context && output_context->pb) {
      avio_closep(&output_context->pb);
    }
    avformat_free_context(output_context);
    av_dict_free(&ffmpeg_options);
    av_freep(&streams);
    LOGE("%s %s %d avformat_write_header failed msg=%s", __FILE_NAME__, __func__ , __LINE__, av_err2str(ret));
    CallOnTransformFailed(listener, -10);
    return;
  }
  AVPacket *packet = av_packet_alloc();
  int64_t last_dts = 0;
  int64_t first_pts_dts = 0;
  int first_pkt = 0;
  while(1) {
    AVStream *in_stream, *out_stream;
    ret = av_read_frame(input_context, packet);
    if (ret < 0) {
      av_packet_unref(packet);
      break;
    }
    if (packet->flags & AV_PKT_FLAG_CORRUPT) {
      if (input_context->flags & AVFMT_FLAG_DISCARD_CORRUPT) {
        av_packet_unref(packet);
        continue;
      }
      av_packet_unref(packet);
      av_packet_free(&packet);
      avformat_close_input(&input_context);
      if (output_context && output_context->pb) {
        avio_closep(&output_context->pb);
      }
      avformat_free_context(output_context);
      av_dict_free(&ffmpeg_options);
      av_freep(&streams);
      LOGE("%s %s %d AVPacket is corrupted", __FILE_NAME__, __func__ , __LINE__);
      CallOnTransformFailed(listener, -11);
      return;
    }
    in_stream = input_context->streams[packet->stream_index];
    if (packet->stream_index >= stream_size ||
        streams[packet->stream_index] < 0) {
      av_packet_unref(packet);
      continue;
    }

    packet->stream_index = streams[packet->stream_index];
    out_stream = output_context->streams[packet->stream_index];

    if (packet->pts == AV_NOPTS_VALUE) {
      if (packet->dts != AV_NOPTS_VALUE) {
        packet->pts = packet->dts;
        last_dts = packet->dts;
      } else {
        packet->pts = last_dts + 1;
        packet->dts = packet->pts;
        last_dts = packet->pts;
      }
    } else {
      if (packet->dts != AV_NOPTS_VALUE) {
        last_dts = packet->dts;
      } else {
        packet->dts = packet->pts;
        last_dts = packet->dts;
      }
    }

    if (packet->pts < packet->dts) {
      packet->pts = packet->dts;
    }

    /// 起始的时间归0
    if (!first_pkt && packet->flags & AV_PKT_FLAG_KEY) {
      first_pts_dts = packet->pts;
      first_pkt++;
    }
    packet->pts = packet->pts - first_pts_dts;
    packet->dts = packet->dts - first_pts_dts;

    if (video_stream_index == packet->stream_index) {
      int64_t video_pts = av_rescale_q(packet->pts, in_stream->time_base, AV_TIME_BASE_Q) / 1000;
      float progress = video_pts * 1.0f * 100 / video_duration;
      if (abs(transform_progress_ - progress) > 0.5f || abs(progress - 100) < 0.1f) {
        CallOnTransformProgress(listener, progress);
        transform_progress_ = progress;
      }
    }

    packet->pts = av_rescale_q_rnd(packet->pts, in_stream->time_base, out_stream->time_base,
                               static_cast<AVRounding>(AV_ROUND_NEAR_INF |
                                   AV_ROUND_PASS_MINMAX));
    packet->dts = av_rescale_q_rnd(packet->dts, in_stream->time_base, out_stream->time_base,
                               static_cast<AVRounding>(AV_ROUND_NEAR_INF |
                                   AV_ROUND_PASS_MINMAX));
    packet->duration = av_rescale_q(packet->duration, in_stream->time_base, out_stream->time_base);
    packet->pos = -1;

    ret = av_interleaved_write_frame(output_context, packet);

    if (ret < 0) {
      av_packet_unref(packet);
      av_packet_free(&packet);
      avformat_close_input(&input_context);
      if (output_context && output_context->pb) {
        avio_closep(&output_context->pb);
      }
      avformat_free_context(output_context);
      av_dict_free(&ffmpeg_options);
      av_freep(&streams);
      LOGE("%s %s %d av_interleaved_write_frame failed msg=%s", __FILE_NAME__, __func__ , __LINE__, av_err2str(ret));
      CallOnTransformFailed(listener, -12);
      return;
    }
    av_packet_unref(packet);
  }
  av_packet_free(&packet);
  ret = av_write_trailer(output_context);
  if (ret == 0) {
    avformat_close_input(&input_context);
    if (output_context && output_context->pb) {
      avio_closep(&output_context->pb);
    }
    avformat_free_context(output_context);
    av_dict_free(&ffmpeg_options);
    av_freep(&streams);
    LOGI("%s %s %d TransformFinished", __FILE_NAME__, __func__ , __LINE__);
    CallOnTransformFinished(listener);
  } else {
    avformat_close_input(&input_context);
    if (output_context && output_context->pb) {
      avio_closep(&output_context->pb);
    }
    avformat_free_context(output_context);
    av_dict_free(&ffmpeg_options);
    av_freep(&streams);
    LOGI("%s %s %d av_write_trailer failed msg=%s", __FILE_NAME__, __func__ , __LINE__, av_err2str(ret));
    CallOnTransformFailed(listener, -13);
  }
}

void VideoProcessor::TransformVideoInternal(const char *config, jobject listener) {

}

void VideoProcessor::CallOnTransformFailed(jobject listener, int err) {
  if (video_process_object_ == nullptr) {
    return;
  }
  JNIEnv *env = nullptr;
  int ret = get_env(&env);
  if (env != nullptr) {
    auto clazz = env->GetObjectClass(video_process_object_);
    auto method = env->GetMethodID(clazz, "onTransformFailed", "(Lcom/jeffmony/m3u8library/listener/IVideoTransformListener;I)V");
    env->CallVoidMethod(video_process_object_, method, listener, err);
    env->DeleteLocalRef(clazz);
    env->DeleteGlobalRef(listener);
  }
  if (ret == JNI_EDETACHED) {
    detach_env();
  }
}

void VideoProcessor::CallOnTransformProgress(jobject listener, float progress) {
  if (video_process_object_ == nullptr) {
    return;
  }
  JNIEnv *env = nullptr;
  int ret = get_env(&env);
  if (env != nullptr) {
    auto clazz = env->GetObjectClass(video_process_object_);
    auto method = env->GetMethodID(clazz, "onTransformProgress", "(Lcom/jeffmony/m3u8library/listener/IVideoTransformListener;F)V");
    env->CallVoidMethod(video_process_object_, method, listener, progress);
    env->DeleteLocalRef(clazz);
  }
  if (ret == JNI_EDETACHED) {
    detach_env();
  }
}

void VideoProcessor::CallOnTransformFinished(jobject listener) {
  if (video_process_object_ == nullptr) {
    return;
  }
  JNIEnv *env = nullptr;
  int ret = get_env(&env);
  if (env != nullptr) {
    auto clazz = env->GetObjectClass(video_process_object_);
    auto method = env->GetMethodID(clazz, "onTransformFinished", "(Lcom/jeffmony/m3u8library/listener/IVideoTransformListener;)V");
    env->CallVoidMethod(video_process_object_, method, listener);
    env->DeleteLocalRef(clazz);
    env->DeleteGlobalRef(listener);
  }
  if (ret == JNI_EDETACHED) {
    detach_env();
  }
}

void VideoProcessor::HandleMessage(thread::Message *msg) {
  int what = msg->what;
  switch (what) {

    case VideoProcessorMessage::kTransformVideo: {
      auto input_path = reinterpret_cast<const char *>(msg->obj1);
      auto output_path = reinterpret_cast<const char *>(msg->obj2);
      auto listener = reinterpret_cast<jobject>(msg->obj3);
      TransformVideoInternal(input_path, output_path, listener);
      delete[] input_path;
      delete[] output_path;
      break;
    }

    case VideoProcessorMessage::kTransformVideoConfig: {
      auto config = reinterpret_cast<const char *>(msg->obj1);
      auto listener = reinterpret_cast<jobject>(msg->obj2);
      TransformVideoInternal(config, listener);
      delete[] config;
      break;
    }
  }
}

void VideoProcessor::RemoveMessage(thread::Message *msg) {

}

} // media