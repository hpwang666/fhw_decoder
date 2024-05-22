//**************************************************************
 
#ifndef _HIK_EVENT_H
#define _HIK_EVENT_H
 
#include "cache.h"
#include "env.h"
typedef struct readJpeg_st *readJpeg_t ;
struct readJpeg_st{
	int bufIndex;
	u_char readBuf[128*1024];
	FILE *fe;
	char fileName[64];
	char *tempHead;
	int fileLength;
	int headLength;
	int contentLen[4];//每个content的长度
	u_char *content[4];  //每个content的起始位置
	loop_ev env;
};

 
int create_event_server(loop_ev);
int free_event_server();
int create_alarm_client(loop_ev env);
 
#endif
