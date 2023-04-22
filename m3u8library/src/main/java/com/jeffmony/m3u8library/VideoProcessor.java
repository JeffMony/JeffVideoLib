package com.jeffmony.m3u8library;

import com.jeffmony.m3u8library.listener.IVideoTransformProgressListener;

public class VideoProcessor {

    private static volatile boolean mIsLibLoaded = false;

    private IVideoTransformProgressListener mListener;

    public static void loadLibrariesOnce() {
        synchronized (VideoProcessor.class) {
            if (!mIsLibLoaded) {
                System.loadLibrary("avcodec");
                System.loadLibrary("avformat");
                System.loadLibrary("avutil");
                System.loadLibrary("swresample");
                System.loadLibrary("swscale");
                System.loadLibrary("media_muxer");
                mIsLibLoaded = true;
                initFFmpegOptions();
            }
        }
    }

    public VideoProcessor() {
        loadLibrariesOnce();
    }

    public static native void initFFmpegOptions();

    //转化视频的封装格式,M3U8 转化为 MP4格式
    public native int transformVideo(String inputPath, String outputPath);

    public void setOnVideoTransformProgressListener(IVideoTransformProgressListener listener) {
        mListener = listener;
    }

    //从native层调用上来,回调当前的视频转化进度
    public void invokeVideoTransformProgress(float progress) {
        if (mListener != null) {
            mListener.onTransformProgress(progress);
        }
    }
}
