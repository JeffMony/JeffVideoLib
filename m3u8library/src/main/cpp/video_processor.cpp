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
#include "process_error.h"
}

AVDictionary *ffmpeg_options = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_jeffmony_m3u8library_VideoProcessor_initFFmpegOptions(JNIEnv *env, jclass clazz) {
    //初始化的时候设置options,可以解决av_dict_set导致的NE问题
    av_dict_set(&ffmpeg_options, PROTOCOL_OPTION_KEY, PROTOCOL_OPTION_VALUE, 0);
    av_dict_set(&ffmpeg_options, FORMAT_EXTENSION_KEY, FORMAT_EXTENSION_VALUE, 0);
}

int64_t get_total_packets_count(const char* in_filename) {
    AVFormatContext  *ifmt_ctx = NULL;
    AVPacket pkt;
    int ret;
    int64_t total_packets_count = 0;
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, &ffmpeg_options)) < 0) {
        LOGE("Could not open input file '%s'", in_filename);
        LOGE("Error occurred: %s\n", av_err2str(ret));
        avformat_close_input(&ifmt_ctx);
        return ret;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, &ffmpeg_options)) < 0) {
        LOGE("Failed to retrieve input stream information");
        LOGE("Error occurred: %s\n", av_err2str(ret));
        avformat_close_input(&ifmt_ctx);
        return ret;
    }

    while(1) {
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) break;
        total_packets_count++;
        av_packet_unref(&pkt);
    }
    avformat_close_input(&ifmt_ctx);
    if (total_packets_count == 0) total_packets_count = 1;
    return total_packets_count;
}

void invoke_video_transform_progress(JNIEnv *env, jobject thiz, float progress) {
    jclass clz = env->GetObjectClass(thiz);
    jmethodID  jmethodId = env->GetMethodID(clz, "invokeVideoTransformProgress", "(F)V");
    env->CallVoidMethod(thiz, jmethodId, progress);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jeffmony_m3u8library_VideoProcessor_transformVideo(JNIEnv *env, jobject thiz, jstring input_path, jstring output_path) {
    if (use_log_report) {
        av_log_set_callback(ffp_log_callback_report);
    } else {
        av_log_set_callback(ffp_log_callback_brief);
    }
    const char *in_filename = env->GetStringUTFChars(input_path, 0);
    const char *out_filename = env->GetStringUTFChars(output_path, 0);
    LOGI("Input_path=%s, Output_path=%s", in_filename, out_filename);
    const AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret, i;
    int stream_index = 0;
    int *stream_mapping = NULL;
    int stream_mapping_size;
    int width = 0, height = 0;
    int64_t last_dts = 0;
    int64_t first_pts_dts = 0;
    int first_pkt = 0;
    int64_t total_packets_count, temp_packets_count = 0;
    float current_progress, last_progress = -1;

    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, &ffmpeg_options)) < 0) {
        LOGE("Could not open input file '%s'", in_filename);
        LOGE("Error occurred: %s\n", av_err2str(ret));
        avformat_close_input(&ifmt_ctx);
        return ret;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, &ffmpeg_options)) < 0) {
        LOGE("Failed to retrieve input stream information");
        LOGE("Error occurred: %s\n", av_err2str(ret));
        avformat_close_input(&ifmt_ctx);
        return ret;
    }

    av_dump_format(ifmt_ctx, 1, in_filename, 0);

    total_packets_count = get_total_packets_count(in_filename);

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        LOGE("Could not create output context\n");
        avformat_close_input(&ifmt_ctx);
        avformat_free_context(ofmt_ctx);
        return ERR_ALLOC_OUTPUT_CTX;
    }
    LOGI("Output format=%s", ofmt_ctx->oformat->name);

//    ofmt_ctx->oformat->flags |= AVFMT_TS_NONSTRICT;
//    ofmt_ctx->oformat->flags |= AVFMT_NODIMENSIONS;

    stream_mapping_size = ifmt_ctx->nb_streams;
    stream_mapping = (int *) av_malloc_array(stream_mapping_size, sizeof(*stream_mapping));
    if (!stream_mapping) {
        LOGE("Could not alloc stream mapping\n");
        avformat_close_input(&ifmt_ctx);
        avformat_free_context(ofmt_ctx);
        av_freep(&stream_mapping);
        return ERR_MALLOC_MAPPING;
    }

    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *out_stream;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }

        if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            width = in_codecpar->width;
            height = in_codecpar->height;

            if (width == 0 && height == 0) {
                LOGE("Input file has no width or height\n");
                avformat_close_input(&ifmt_ctx);
                avformat_free_context(ofmt_ctx);
                av_freep(&stream_mapping);
                return ERR_DIMENSIONS_NOT_SET;
            }
        }

        stream_mapping[i] = stream_index++;

        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            LOGE("Failed allocating output stream\n");
            avformat_close_input(&ifmt_ctx);
            avformat_free_context(ofmt_ctx);
            av_freep(&stream_mapping);
            return ERR_CREATE_NEW_STREAM;
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            LOGE("Failed to copy codec parameters\n");
            LOGE("Error occurred: %s\n", av_err2str(ret));
            avformat_close_input(&ifmt_ctx);
            avformat_free_context(ofmt_ctx);
            av_freep(&stream_mapping);
            return ret;
        }
        out_stream->codecpar->codec_tag = 0;
    }

    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    if (!(ofmt->flags & AVFMT_NOFILE)) {
        LOGI("Open output file");
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGE("Could not open '%s'", out_filename);
            LOGE("Error occurred: %s\n, width=%d, height=%d", av_err2str(ret), width, height);
            avformat_close_input(&ifmt_ctx);
            /* close output */
            if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
                avio_closep(&ofmt_ctx->pb);
            avformat_free_context(ofmt_ctx);
            av_freep(&stream_mapping);
            return ret;
        }
    }

    ret = avformat_write_header(ofmt_ctx, &ffmpeg_options);
    if (ret < 0) {
        LOGE("Error occurred when opening output file, ret=%d\n", ret);
        LOGE("Error occurred: %s\n, width=%d, height=%d", av_err2str(ret), width, height);
        avformat_close_input(&ifmt_ctx);
        /* close output */
        if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
            avio_closep(&ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        av_freep(&stream_mapping);
        return ret;
    }

    while (1) {
        AVStream *in_stream, *out_stream;

        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index >= stream_mapping_size ||
            stream_mapping[pkt.stream_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }

        pkt.stream_index = stream_mapping[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];

        if (pkt.pts == AV_NOPTS_VALUE) {
            if (pkt.dts != AV_NOPTS_VALUE) {
                pkt.pts = pkt.dts;
                last_dts = pkt.dts;
            } else {
                pkt.pts = last_dts + 1;
                pkt.dts = pkt.pts;
                last_dts = pkt.pts;
            }
        } else {
            if (pkt.dts != AV_NOPTS_VALUE) {
                last_dts = pkt.dts;
            } else {
                pkt.dts = pkt.pts;
                last_dts = pkt.dts;
            }
        }

        if (pkt.pts < pkt.dts) {
            pkt.pts = pkt.dts;
        }

        //起始的时间归0
        if (!first_pkt && pkt.flags & AV_PKT_FLAG_KEY) {
            first_pts_dts = pkt.pts;
            first_pkt++;
        }

        pkt.pts = pkt.pts - first_pts_dts;
        pkt.dts = pkt.dts - first_pts_dts;

        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF |
                                                           AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF |
                                                           AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);

        temp_packets_count++;
        current_progress = temp_packets_count * 1.0f * 100 / total_packets_count;
        //防止JNI回调过分频繁,进行一些代码逻辑上的限制
        if (abs(current_progress - last_progress) > 0.5f || abs(current_progress - 100) < 0.1f) {
            invoke_video_transform_progress(env, thiz, current_progress);
            last_progress = current_progress;
        }

        if (ret < 0) {
            LOGE("Error muxing packet\n");
            avformat_close_input(&ifmt_ctx);
            /* close output */
            if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
                avio_closep(&ofmt_ctx->pb);
            avformat_free_context(ofmt_ctx);
            av_freep(&stream_mapping);
            return ret;
        }
        av_packet_unref(&pkt);
    }

    av_write_trailer(ofmt_ctx);

    avformat_close_input(&ifmt_ctx);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    av_freep(&stream_mapping);
    return 1;
}