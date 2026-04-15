#ifndef RTSP_READER_H
#define RTSP_READER_H

#include <libavformat/avformat.h>

typedef struct {
    AVFormatContext *fmt_ctx;
    char url[512];
    int video_stream_idx;
    int audio_stream_idx;
    int reconnect_delay;
} RTSPReader;

int rtsp_reader_init(RTSPReader *reader, const char *url);
int rtsp_reader_open(RTSPReader *reader);
int rtsp_reader_read_packet(RTSPReader *reader, AVPacket *pkt);
void rtsp_reader_close(RTSPReader *reader);
void rtsp_reader_cleanup(RTSPReader *reader);

#endif
