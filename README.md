# JeffM3U8Lib
M3U8视频合并为MP4视频专用库,已经尽量裁减优化,减少包大小,只保留这个功能
> * 尽量将so包裁减到最小了
> * 提供M3U8合成MP4的合成进度
> * 解决了合成过程中的pts和dts异常问题

#### 怎么做裁减优化
> * 明确M3U8合成MP4所需要支持的格式
> * 去掉不用的模块和格式
> * 解决合成过程中的异常问题,主要是pts和dts的异常问题

