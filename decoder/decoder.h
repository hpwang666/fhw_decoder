#ifndef _DECODER_H
#define _DECODER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include "common.h"
#include "dec_buf.h"
#include "vdec.h"

#define data_abs(a,b) ((a>=b)?(a-b):(b-a))
typedef struct decoder_st *decoder_t;
typedef struct rtpPkg_st  *rtpPkg_t;
typedef struct rtspChnStatus_st *rtspChnStatus_t;
typedef struct decEnv_st *decEnv_t; 
#define PKG_LIST_LEN  (6)


typedef enum{PKG_BAD=-1,PKG_MIN=0,PKG_TXT=1} PKG_TYPE;

#define  VDEC_CHN_NUM_25 (25)
#define  VDEC_CHN_NUM_16 (16)
#define  VDEC_CHN_NUM_4 (4)


#define CHNS (VDEC_CHN_NUM_16)


extern decEnv_t  decEnv;
struct decoder_st{
	int 	id;
	unsigned int timestamp;
	unsigned long long time40ms;
	unsigned short last_seqc;
	VDEC_STREAM_S decStream;
	dec_buf_t 	buf;
	//rtpPkg_t   rtpPkgList;
	pthread_mutex_t decLock;
	unsigned EN_PPS:1;//
	unsigned EN_SPS:1;//用来标志 SPS PPS 只需要传输一次
	unsigned PKG_STARTED:1;			//BUFER 是否接收到了  包头
	unsigned err:1;  				//表示当前画面为无视频状态，当有视频流来的时候，应该清除该标志
	unsigned startRcv:1;			//表示开始接收码流
	unsigned refused:1;				//表示此时不接受视频
	unsigned waitIfream:1;
};

struct rtpPkg_st
{
	char magic[8];
	int cmd;
	int len;
	u_char data[1600];
};

struct rtspChnStatus_st{ //和RTSPclient通信的接口，当不是RTSP数据包的时候就会出现这个包
	int id;
	int err;
	int update; //表示该通道切换了视频源，可以进行reset操作
	int subwin;
};


struct decEnv_st{
	decoder_t dec25;
};

decEnv_t create_dec_chns(void);

void free_dec_chns(decEnv_t);

#endif
