#ifndef _TIMER_H
#define _TIMER_H

#include "core.h"

#define timer_abs(value)       (((value) >= 0) ? (value) : - (value))

#define TIMER_LAZY_DELAY 200

//这个能判断最小时间间隔的分辨率 为  add(ev,timer)  timer - TIMER_LAZY_DELAY > 包正常的间隔

//比如在RTSP包里面，有的包因网络原因自然间隔就达到 500ms  timer 就至少要选700，否则就会触发超时事件

#ifdef __cplusplus
extern "C"
{
#endif

void time_update(void);
int init_timer(void);
int free_timer();

void expire_timers(void);
int del_timer(event_t ev);
int add_timer(event_t ev, msec64 timer);
msec64 find_timer(void);

#ifdef __cplusplus
}
#endif

#endif
