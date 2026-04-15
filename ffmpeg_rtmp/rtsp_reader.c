#include "rtsp_reader.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int rtsp_reader_init(RTSPReader *reader, const char *url) {
    memset(reader, 0, sizeof(RTSPReader));
    strncpy(reader->url, url, sizeof(reader->url) - 1);
    reader->video_stream_idx = -1;
    reader->audio_stream_idx = -1;
    reader->reconnect_delay = 3;
    return 0;
}

int rtsp_reader_open(RTSPReader *reader) {
    // 调试：检查是否支持 rtsp demuxer
    if (!av_find_input_format("rtsp")) {
        fprintf(stderr, "FFmpeg 未编译 RTSP demuxer 支持，请检查 FFmpeg 编译配置\n");
        return AVERROR_DEMUXER_NOT_FOUND;
    }

    AVDictionary *opts = NULL;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "stimeout", "5000000", 0);
    av_dict_set(&opts, "fflags", "nobuffer", 0);        // 禁用输入缓冲
    av_dict_set(&opts, "flags", "low_delay", 0);        // 低延迟模式
    av_dict_set(&opts, "max_delay", "500000", 0);       // 最大延迟 0.5s
    av_dict_set(&opts, "reorder_queue_size", "0", 0);   // 禁用重排序队列
    
    int ret = avformat_open_input(&reader->fmt_ctx, reader->url, NULL, &opts);
    av_dict_free(&opts);
    
    if (ret < 0) {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        fprintf(stderr, "无法打开RTSP流: %s, 错误: %s\n", reader->url, errbuf);
        return ret;
    }
    
    ret = avformat_find_stream_info(reader->fmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "无法获取流信息\n");
        avformat_close_input(&reader->fmt_ctx);
        return ret;
    }
    
    for (unsigned int i = 0; i < reader->fmt_ctx->nb_streams; i++) {
        AVCodecParameters *codecpar = reader->fmt_ctx->streams[i]->codecpar;
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO && reader->video_stream_idx < 0) {
            reader->video_stream_idx = i;
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO && reader->audio_stream_idx < 0) {
            reader->audio_stream_idx = i;
        }
    }
    
    printf("RTSP连接成功: %s\n", reader->url);
    return 0;
}

int rtsp_reader_read_packet(RTSPReader *reader, AVPacket *pkt) {
    return av_read_frame(reader->fmt_ctx, pkt);
}

void rtsp_reader_close(RTSPReader *reader) {
    if (reader->fmt_ctx) {
        avformat_close_input(&reader->fmt_ctx);
    }
}

void rtsp_reader_cleanup(RTSPReader *reader) {
    rtsp_reader_close(reader);
}
