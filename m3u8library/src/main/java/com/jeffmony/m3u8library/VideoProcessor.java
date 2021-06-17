package com.jeffmony.m3u8library;

public class VideoProcessor {

    private static volatile boolean mIsLibLoaded = false;

    public static void loadLibrariesOnce() {
        synchronized (VideoProcessor.class) {
            if (!mIsLibLoaded) {
                System.loadLibrary("avcodec");
                System.loadLibrary("avformat");
                System.loadLibrary("avutil");
                System.loadLibrary("swresample");
                System.loadLibrary("swscale");
                System.loadLibrary("jeffmony");

                mIsLibLoaded = true;

                initFFmpegOptions();
            }
        }
    }

    public VideoProcessor() {
        loadLibrariesOnce();
    }

    public static native void initFFmpegOptions();
}
