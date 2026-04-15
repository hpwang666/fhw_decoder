#ifndef STREAM_CONVERTER_H
#define STREAM_CONVERTER_H

#include "rtsp_reader.h"
#include "rtmp_writer.h"
#include "packet_queue.h"
#include <pthread.h>

typedef struct {
    RTSPReader reader;
    RTMPWriter writer;
    PacketQueue queue;
    pthread_t rtsp_thread;
    pthread_t rtmp_thread;
    int running;
    char rtsp_url[512];
    char rtmp_url[512];
} StreamConverter;

int stream_converter_init(StreamConverter *conv, const char *rtsp_url, const char *rtmp_url);
int stream_converter_run(StreamConverter *conv);
void stream_converter_stop(StreamConverter *conv);
void stream_converter_cleanup(StreamConverter *conv);

#endif
