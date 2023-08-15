package com.jeffmony.m3u8library.listener;

public interface IVideoTransformListener {

    void onTransformProgress(float progress);

    void onTransformFailed(int err);

    void onTransformFinished();
}
