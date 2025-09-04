#ifndef _PLC_BUS_H
#define _PLC_BUS_H
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>


#include "connet.h"

typedef struct modbusHeader_st *modbusHeader_t;
typedef struct modbusSession_st *modbusSession_t;
struct modbusHeader_st{
	u_char context[2];
	u_char protocol[2];
	uint16_t length;
	u_char flag;
	u_char opt;//功能码
	uint16_t optLen;//操作的数据长度
	uint16_t index;//操作的起始偏移量
};

struct modbusSession_st{
	modbusHeader_t modbusHeader;
	event_t modbusEv;
	buf_t sendBuf;
	u_char holdReg[255];//保持寄存器数据
	uint16_t crc;   		//寄存器10个字节校验值
};



int initPlcBus();
int releasePlcBus();

#endif
