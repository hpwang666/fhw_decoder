#ifndef _ENV_H
#define _ENV_H

#include "queue.h"
#include "httpclient.h"
#include "connet.h"
#include "zlog.h"

struct custom_st{
	int ch;
	int cmd;
	int stop;
};
typedef struct custom_st *custom_t;

struct _camConnection
{
	char address[32];
	char url[96];
	int channel;
	httpclient_t ct;
	char camName[32];
};

typedef struct _camConnection *camConnection;


struct event2plc_st{
	int ch;
	int event;//0表示没有事件
};
typedef struct event2plc_st *event2plc_t;

struct _loop_ev
{

	zlog_category_t *zc;
	char plcCams[12][64];
	camConnection camConn;
	int ch0_elevation;//初始化的时候保存摄像头垂直位置
	int ch0_azimuth;//初始化的时候保存摄像头水平位置
	queue_t ptzQueue;
	queue_t voQueue;
	poolList_t poolList;
	int serialFd;

	int channel;//当进行多画面操作的时候，必须置位-1
	int voType;// 多画面显示 1/4/6/9/16
	int lastCmd; //存储上一次的指令

	int muxt4;//4画面时显示子码流还是主码流
	int decType;//0--h264 1--h265
	char passwd[32];

	char plcAddr[32]; //PLC的IP地址
	int plcPort; //plc的端口
	int r0;  //寄存器0
	int r1;  //寄存器1
	int protocol;  //plc总线协议
	conn_t modbusListenConn;

	char alarm_ip[32];//报警主机IP
	int alarmPort;//报警主机端口
	int event_type;//事件类型 0--人脸  1--人体   2--机动车  3--火灾
	queue_t eventQueue;//事件推送
	conn_t alarmConn;


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
