#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include <libavformat/avformat.h>
#include <pthread.h>

#define MAX_QUEUE_SIZE 100

typedef struct {
    AVPacket packets[MAX_QUEUE_SIZE];
    int read_pos;
    int write_pos;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond_not_empty;
    pthread_cond_t cond_not_full;
    int abort_request;
} PacketQueue;

int packet_queue_init(PacketQueue *q);
int packet_queue_put(PacketQueue *q, AVPacket *pkt);
int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);
void packet_queue_flush(PacketQueue *q);
void packet_queue_abort(PacketQueue *q);
void packet_queue_destroy(PacketQueue *q);

#endif
