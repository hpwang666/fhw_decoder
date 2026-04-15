#include "stream_converter.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void* rtsp_thread_func(void *arg) {
    StreamConverter *conv = (StreamConverter*)arg;
    AVPacket pkt;
    int ret;
    
    printf("RTSP读取线程已启动\n");
    
    while (conv->running) {
        ret = rtsp_reader_open(&conv->reader);
        if (ret < 0) {
            fprintf(stderr, "RTSP连接失败，%d秒后重试...\n", conv->reader.reconnect_delay);
            sleep(conv->reader.reconnect_delay);
            continue;
        }
        
        while (conv->running) {
            ret = rtsp_reader_read_packet(&conv->reader, &pkt);
            
            if (ret < 0) {
                fprintf(stderr, "RTSP读取失败，准备重连...\n");
                break;
            }
            
            // 将数据包放入队列
            if (packet_queue_put(&conv->queue, &pkt) < 0) {
                av_packet_unref(&pkt);
                break;
            }
        }
        
        rtsp_reader_close(&conv->reader);
        
        if (conv->running) {
            fprintf(stderr, "等待 %d 秒后重连RTSP...\n", conv->reader.reconnect_delay);
            packet_queue_flush(&conv->queue);
            sleep(conv->reader.reconnect_delay);
        }
    }
    
    printf("RTSP读取线程已退出\n");
    return NULL;
}

static void* rtmp_thread_func(void *arg) {
    StreamConverter *conv = (StreamConverter*)arg;
    AVPacket pkt;
    int ret;
    int rtmp_opened = 0;
    
    printf("RTMP写入线程已启动\n");
    
    // 等待RTSP连接成功
    sleep(2);
    
    while (conv->running) {
        if (!rtmp_opened && conv->reader.fmt_ctx) {
            ret = rtmp_writer_open(&conv->writer, conv->reader.fmt_ctx);
            if (ret < 0) {
                fprintf(stderr, "RTMP推流失败，1秒后重试...\n");
                sleep(1);
                continue;
            }
            rtmp_opened = 1;
        }
        
        if (!rtmp_opened) {
            sleep(1);
            continue;
        }
        
        ret = packet_queue_get(&conv->queue, &pkt, 1);
        if (ret < 0) {
            continue;
        }
        
        AVStream *in_stream = conv->reader.fmt_ctx->streams[pkt.stream_index];
        ret = rtmp_writer_write_packet(&conv->writer, &pkt, in_stream, pkt.stream_index);
        
        av_packet_unref(&pkt);
        
        if (ret < 0) {
            fprintf(stderr, "RTMP写入失败\n");
        }
        
        rtmp_writer_check_congestion(&conv->writer);
    }
    
    printf("RTMP写入线程已退出\n");
    return NULL;
}

int stream_converter_init(StreamConverter *conv, const char *rtsp_url, const char *rtmp_url) {
    memset(conv, 0, sizeof(StreamConverter));

    strncpy(conv->rtsp_url, rtsp_url, sizeof(conv->rtsp_url) - 1);
    strncpy(conv->rtmp_url, rtmp_url, sizeof(conv->rtmp_url) - 1);

    rtsp_reader_init(&conv->reader, rtsp_url);
    rtmp_writer_init(&conv->writer, rtmp_url);
    packet_queue_init(&conv->queue);

    conv->running = 1;

    return 0;
}

int stream_converter_run(StreamConverter *conv) {
    pthread_create(&conv->rtsp_thread, NULL, rtsp_thread_func, conv);
    pthread_create(&conv->rtmp_thread, NULL, rtmp_thread_func, conv);
    
    pthread_join(conv->rtsp_thread, NULL);
    pthread_join(conv->rtmp_thread, NULL);
    
    return 0;
}

void stream_converter_stop(StreamConverter *conv) {
    conv->running = 0;
    packet_queue_abort(&conv->queue);
}

void stream_converter_cleanup(StreamConverter *conv) {
    packet_queue_destroy(&conv->queue);
    rtsp_reader_cleanup(&conv->reader);
    rtmp_writer_cleanup(&conv->writer);
}
