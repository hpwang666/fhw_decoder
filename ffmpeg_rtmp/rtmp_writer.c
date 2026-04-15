#include "rtmp_writer.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

static int64_t get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int rtmp_writer_init(RTMPWriter *writer, const char *url) {
    memset(writer, 0, sizeof(RTMPWriter));
    strncpy(writer->url, url, sizeof(writer->url) - 1);
    writer->congestion_threshold_ms = 1000;
    writer->last_write_time = get_time_ms();
    return 0;
}

int rtmp_writer_open(RTMPWriter *writer, AVFormatContext *input_ctx) {
    int ret = avformat_alloc_output_context2(&writer->fmt_ctx, NULL, "flv", writer->url);
    if (ret < 0) {
        fprintf(stderr, "无法创建输出上下文\n");
        return ret;
    }
    
    for (unsigned int i = 0; i < input_ctx->nb_streams; i++) {
        AVStream *in_stream = input_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(writer->fmt_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "无法创建输出流\n");
            return AVERROR_UNKNOWN;
        }
        
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            fprintf(stderr, "无法复制编解码器参数\n");
            return ret;
        }
        out_stream->codecpar->codec_tag = 0;
    }
    
    if (!(writer->fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&writer->fmt_ctx->pb, writer->url, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "无法打开输出URL: %s\n", writer->url);
            return ret;
        }
    }
    
    ret = avformat_write_header(writer->fmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "写入头部失败\n");
        return ret;
    }

    // 禁用输出缓冲，降低推流延迟
    if (writer->fmt_ctx->pb) {
        writer->fmt_ctx->pb->direct = 1;
    }
    writer->fmt_ctx->max_interleave_delta = 0;  // 禁用交错缓冲
    
    printf("RTMP推流已启动: %s\n", writer->url);
    return 0;
}

int rtmp_writer_write_packet(RTMPWriter *writer, AVPacket *pkt, AVStream *in_stream, int stream_idx) {
    int64_t start_time = get_time_ms();

    AVStream *out_stream = writer->fmt_ctx->streams[stream_idx];

    av_packet_rescale_ts(pkt, in_stream->time_base, out_stream->time_base);
    pkt->stream_index = stream_idx;
    pkt->pos = -1;

    // 保证 DTS 单调递增
    if (pkt->dts != AV_NOPTS_VALUE) {
        if (pkt->dts <= writer->last_dts[stream_idx]) {
            pkt->dts = writer->last_dts[stream_idx] + 1;
        }
        writer->last_dts[stream_idx] = pkt->dts;
    }
    // PTS 不能小于 DTS
    if (pkt->pts != AV_NOPTS_VALUE && pkt->dts != AV_NOPTS_VALUE) {
        if (pkt->pts < pkt->dts)
            pkt->pts = pkt->dts;
    }

    int ret = av_write_frame(writer->fmt_ctx, pkt);
    
    int64_t write_duration = get_time_ms() - start_time;
    if (write_duration > writer->congestion_threshold_ms) {
        fprintf(stderr, "警告: RTMP写入延迟 %lld ms，可能发生拥塞\n", 
                (long long)write_duration);
    }
    
    writer->last_write_time = get_time_ms();
    return ret;
}

int rtmp_writer_check_congestion(RTMPWriter *writer) {
    int64_t now = get_time_ms();
    int64_t elapsed = now - writer->last_write_time;
    
    if (elapsed > writer->congestion_threshold_ms * 3) {
        fprintf(stderr, "严重: RTMP推流可能已阻塞 (%lld ms 无写入)\n", 
                (long long)elapsed);
        return 1;
    }
    return 0;
}

void rtmp_writer_close(RTMPWriter *writer) {
    if (writer->fmt_ctx) {
        av_write_trailer(writer->fmt_ctx);
        if (!(writer->fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&writer->fmt_ctx->pb);
        }
        avformat_free_context(writer->fmt_ctx);
        writer->fmt_ctx = NULL;
    }
}

void rtmp_writer_cleanup(RTMPWriter *writer) {
    rtmp_writer_close(writer);
}
