# RTSP to RTMP 转推流工具

基于 FFmpeg API 的 RTSP 拉流转 RTMP 推流工具，支持断流重连和拥塞检测。

## 功能特性

- RTSP 拉流（TCP 传输）
- RTMP 推流
- 双线程架构（RTSP读取和RTMP写入分离）
- 环形缓冲队列（最大100个数据包）
- 自动断流重连（3秒间隔）
- RTMP 拥塞检测和提示
- 信号处理（优雅退出）

## 架构说明

程序采用生产者-消费者模式：
- RTSP线程：负责拉流并将数据包放入队列
- RTMP线程：从队列取出数据包并推流
- 线程间通过互斥锁和条件变量同步
- 队列满时RTSP线程阻塞，队列空时RTMP线程阻塞

## 依赖安装

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y libavformat-dev libavcodec-dev libavutil-dev libswscale-dev
```

### CentOS/RHEL
```bash
sudo yum install -y ffmpeg-devel
```

## 编译

```bash
make
```

## 使用方法

```bash
./rtsp2rtmp <rtsp_url> <rtmp_url>
```

### 示例

```bash
./rtsp2rtmp rtsp://192.168.1.100:554/stream rtmp://live.example.com/app/streamkey
```

## 清理

```bash
make clean
```

## 注意事项

- 确保 RTSP 源可访问
- 确保 RTMP 服务器已配置并接受推流
- 程序会自动处理 RTSP 断流重连
- 当 RTMP 写入延迟超过 1 秒时会提示拥塞警告
