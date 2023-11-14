package com.jeffmony.m3u8library;

import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;

import com.jeffmony.m3u8library.listener.IVideoTransformListener;

import java.io.File;

public class VideoProcessManager {

    private static volatile VideoProcessManager sInstance = null;
    private static volatile boolean sLoadLibrary = false;
    private final Handler mMainHandler = new Handler(Looper.getMainLooper());
    private long mHandler = 0;

    private static void loadLibrary() {
        if (sLoadLibrary) {
            return;
        }
        System.loadLibrary("message_queue");
        System.loadLibrary("media_muxer");
        sLoadLibrary = true;
    }

    public static VideoProcessManager getInstance() {
        if (sInstance == null) {
            synchronized (VideoProcessManager.class) {
                if (sInstance == null) {
                    loadLibrary();
                    sInstance = new VideoProcessManager();
                }
            }
        }
        return sInstance;
    }

    private VideoProcessManager() {
        mHandler = createHandler();
    }

    public void transformM3U8ToMp4(final String inputPath, final String outputPath, final IVideoTransformListener listener) {
        if (listener == null) {
            return;
        }
        if (TextUtils.isEmpty(inputPath) || TextUtils.isEmpty(outputPath)) {
            listener.onTransformFailed(-1);
            return;
        }
        final File inputFile = new File(inputPath);
        if (!inputFile.exists()) {
            listener.onTransformFailed(-2);
            return;
        }
        transformM3U8ToMp4(mHandler, inputPath, outputPath, listener);
    }

    public void transformToMp4(final String config, final IVideoTransformListener listener) {
        if (listener == null) {
            return;
        }
        if (TextUtils.isEmpty(config)) {
            listener.onTransformFailed(-1);
            return;
        }
        transformToMp4(mHandler, config, listener);
    }

    private void onTransformProgress(IVideoTransformListener listener, float progress) {
        if (listener == null) {
            return;
        }
        mMainHandler.post(() -> {
            if (listener != null) {
                listener.onTransformProgress(progress);
            }
        });
    }

    private void onTransformFinished(IVideoTransformListener listener) {
        if (listener == null) {
            return;
        }
        mMainHandler.post(() -> {
           if (listener != null) {
               listener.onTransformFinished();
           }
        });
    }

    private void onTransformFailed(IVideoTransformListener listener, int err) {
        if (listener == null) {
            return;
        }
        mMainHandler.post(() -> {
           if (listener != null) {
               listener.onTransformFailed(err);
           }
        });
    }

    private native long createHandler();
    private native void transformM3U8ToMp4(long handler, String inputPath, String outputPath, IVideoTransformListener listener);
    private native void transformToMp4(long handler, String config, IVideoTransformListener listener);
}
