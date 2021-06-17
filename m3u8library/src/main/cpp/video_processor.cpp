//
// Created by JeffMony Lee on 2021/06/17.
//

#include <jni.h>
#include <string>
#include <complex.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/mathematics.h"
#include "libavutil/timestamp.h"
#include "android_log.h"
#include "ffmpeg_options_const.h"
}

AVDictionary *ffmpeg_options = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_jeffmony_m3u8library_VideoProcessor_initFFmpegOptions(JNIEnv *env, jclass clazz) {
    //初始化的时候设置options,可以解决av_dict_set导致的NE问题
    av_dict_set(&ffmpeg_options, PROTOCOL_OPTION_KEY, PROTOCOL_OPTION_VALUE, 0);
    av_dict_set(&ffmpeg_options, FORMAT_EXTENSION_KEY, FORMAT_EXTENSION_VALUE, 0);
}