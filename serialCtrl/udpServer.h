#ifndef _UDP_SERVER_H
#define _UDP_SERVER_H
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

 
#include "connet.h"

int init_udp_conn(conn_t c,void *arg);

#endif
