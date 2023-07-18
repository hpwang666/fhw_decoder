#ifndef _VO_PROCESS_H
#define _VO_PROCESS_H
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>


#include "connet.h"
#include "env.h"
int rtsp_connect_handler(event_t ev);
int rtsp_reconnect_peer(event_t ev);
int init_udp_conn(conn_t c,void *arg);
int transVo(conn_t , custom_t );
#endif
