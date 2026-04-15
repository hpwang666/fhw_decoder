#include "packet_queue.h"
#include <string.h>

int packet_queue_init(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond_not_empty, NULL);
    pthread_cond_init(&q->cond_not_full, NULL);
    return 0;
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    pthread_mutex_lock(&q->mutex);

    if (q->abort_request) {
        pthread_mutex_unlock(&q->mutex);
        return -1;
    }

    // 队列满时丢弃最旧的包，保持低延迟
    if (q->count >= MAX_QUEUE_SIZE) {
        av_packet_unref(&q->packets[q->read_pos]);
        q->read_pos = (q->read_pos + 1) % MAX_QUEUE_SIZE;
        q->count--;
    }

    av_packet_move_ref(&q->packets[q->write_pos], pkt);
    q->write_pos = (q->write_pos + 1) % MAX_QUEUE_SIZE;
    q->count++;

    pthread_cond_signal(&q->cond_not_empty);
    pthread_mutex_unlock(&q->mutex);

    return 0;
}

int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
    pthread_mutex_lock(&q->mutex);
    
    while (q->count == 0 && !q->abort_request && block) {
        pthread_cond_wait(&q->cond_not_empty, &q->mutex);
    }
    
    if (q->abort_request || q->count == 0) {
        pthread_mutex_unlock(&q->mutex);
        return -1;
    }
    
    av_packet_move_ref(pkt, &q->packets[q->read_pos]);
    q->read_pos = (q->read_pos + 1) % MAX_QUEUE_SIZE;
    q->count--;
    
    pthread_cond_signal(&q->cond_not_full);
    pthread_mutex_unlock(&q->mutex);
    
    return 0;
}

void packet_queue_flush(PacketQueue *q) {
    pthread_mutex_lock(&q->mutex);
    
    for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
        av_packet_unref(&q->packets[i]);
    }
    q->read_pos = 0;
    q->write_pos = 0;
    q->count = 0;
    
    pthread_mutex_unlock(&q->mutex);
}

void packet_queue_abort(PacketQueue *q) {
    pthread_mutex_lock(&q->mutex);
    q->abort_request = 1;
    pthread_cond_broadcast(&q->cond_not_empty);
    pthread_cond_broadcast(&q->cond_not_full);
    pthread_mutex_unlock(&q->mutex);
}

void packet_queue_destroy(PacketQueue *q) {
    packet_queue_flush(q);
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond_not_empty);
    pthread_cond_destroy(&q->cond_not_full);
}
