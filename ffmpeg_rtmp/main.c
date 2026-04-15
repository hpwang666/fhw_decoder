#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "stream_converter.h"

static StreamConverter g_converter;

void signal_handler(int sig) {
    printf("\n收到信号 %d，正在停止...\n", sig);
    stream_converter_stop(&g_converter);
}

int main(int argc, char *argv[]) {
    // if (argc != 3) {
    //     fprintf(stderr, "用法: %s <rtsp_url> <rtmp_url>\n", argv[0]);
    //     fprintf(stderr, "示例: %s rtsp://192.168.1.100:554/stream rtmp://live.example.com/app/stream\n", argv[0]);
    //     return 1;
    // }
    
    const char *rtsp_url = "rtsp://admin:fhjt12345@192.168.1.44:554/h264/ch1/main/av_stream";
    const char *rtmp_url = "rtmp://www.07175858.com:8082/live/fuckU";
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("RTSP到RTMP转推流程序\n");
    printf("输入: %s\n", rtsp_url);
    printf("输出: %s\n", rtmp_url);
    
    stream_converter_init(&g_converter, rtsp_url, rtmp_url);
    
    int ret = stream_converter_run(&g_converter);
    
    stream_converter_cleanup(&g_converter);
    
    printf("程序已退出\n");
    return ret;
}
