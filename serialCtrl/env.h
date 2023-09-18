#ifndef _ENV_H
#define _ENV_H

#include "queue.h"
#include "httpclient.h"


struct custom_st{
	int ch;
	int cmd;
	int stop;
};
typedef struct custom_st *custom_t;

struct _camConnection
{
	char address[32];
	int channel;
	httpclient_t ct;
	char camName[32];
};

typedef struct _camConnection *camConnection;

struct _loop_ev
{
	char plcCams[3][32];
	camConnection camConn;
	queue_t ptzQueue;
	queue_t voQueue;
	int serialFd;

	int channel;//当进行多画面操作的时候，必须置位-1
	int voType;// 多画面显示 1/4/6/9/16
	int lastCmd; //存储上一次的指令

	int muxt4;//4画面时显示子码流还是主码流
	int decType;//0--h264 1--h265
	
	int uart_running;
	int vo_running;
	int http_running;
	
	pthread_t uart_worker;
	pthread_t vo_worker;
	pthread_t http_worker;
	
	pthread_mutex_t mutex;
	pthread_mutex_t mutex_cmd;
};

typedef struct _loop_ev *loop_ev;


#endif
