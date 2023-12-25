#ifndef _HIKISAPI_H
#define _HIKISAPI_H

#include "queue.h"
#include "httpclient.h"
#include "env.h"

typedef enum {WIPER=0,ZOOMIN,ZOMMOUT,UP,DOWN,LEFT,RIGHT} camCommand;
typedef enum {START,STOP} doORnot;



int getChnnelInfo(loop_ev ev);
int getCamName(loop_ev,camConnection camConn);
int getPtzStatus(loop_ev ev,camConnection camConn);
#endif
