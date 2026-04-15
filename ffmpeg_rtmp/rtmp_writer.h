#ifndef RTMP_WRITER_H
#define RTMP_WRITER_H

#include <libavformat/avformat.h>

typedef struct {
    AVFormatContext *fmt_ctx;
    char url[512];
    int64_t last_write_time;
    int congestion_threshold_ms;
    int64_t last_dts[32];  // 每路流的上一个 DTS，保证单调递增
} RTMPWriter;

int rtmp_writer_init(RTMPWriter *writer, const char *url);
int rtmp_writer_open(RTMPWriter *writer, AVFormatContext *input_ctx);
int rtmp_writer_write_packet(RTMPWriter *writer, AVPacket *pkt, AVStream *in_stream, int stream_idx);
int rtmp_writer_check_congestion(RTMPWriter *writer);
void rtmp_writer_close(RTMPWriter *writer);
void rtmp_writer_cleanup(RTMPWriter *writer);

#endif
