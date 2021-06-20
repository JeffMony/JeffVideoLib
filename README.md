# JeffM3U8Lib
M3U8视频合并为MP4视频专用库,已经尽量裁减优化,减少包大小,只保留这个功能
> * 尽量将so包裁减到最小了
> * 提供M3U8合成MP4的合成进度
> * 解决了合成过程中的pts和dts异常问题

#### 怎么做裁减优化
> * 明确M3U8合成MP4所需要支持的格式
> * 去掉不用的模块和格式
> * 解决合成过程中的异常问题,主要是pts和dts的异常问题

具体参考[https://github.com/JeffMony/AndroidFFmpegCompile](https://github.com/JeffMony/AndroidFFmpegCompile)工程
```
--disable-gpl \
--disable-doc \
--disable-static \
--disable-x86asm \
--disable-asm \
--disable-symver \
--disable-devices \
--disable-avdevice \
--disable-postproc \
--disable-avfilter \
--disable-avresample \
--disable-ffmpeg \
--disable-ffplay \
--disable-ffprobe \
--disable-programs \
--disable-encoders \
--disable-decoders \
--enable-decoder=h264 \
--enable-decoder=aac \
--enable-decoder=mp3 \
--disable-muxers \
--enable-muxer=mp4 \
--disable-demuxers \
--enable-demuxer=hls \
--enable-demuxer=mov \
--enable-demuxer=mpegts \
```

#### 如何接入这个SDK
目前最新的版本号是1.0.0，如果需要在自己项目中集成，需要做好特定的依赖。<br>
在build.gradle中引入
```
allprojects {
    repositories {
	    maven { url 'https://jitpack.io' }
	}
}
```
在你的app中直接引用
```
dependencies {
    implementation 'com.github.JeffMony:JeffM3U8Lib:1.0.0'
}
```

#### 如何调用这个SDK
传入的M3U8路径需要是本地文件路径，如何下载这个M3U8文件，可以参考开源项目：<br>
[https://github.com/JeffMony/VideoDownloader](https://github.com/JeffMony/VideoDownloader)
```
VideoProcessManager.getInstance().transformM3U8ToMp4(final String inputFilePath, final String outputFilePath, @NonNull final IVideoTransformListener listener)


public interface IVideoTransformListener {

    void onTransformProgress(float progress);

    void onTransformFailed(Exception e);

    void onTransformFinished();
}
```
