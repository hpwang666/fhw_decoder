#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <time.h>
#include <sys/socket.h>
#include <fcntl.h>  

#include "decoder.h"



decEnv_t create_dec_chns(void)
{
	int i=0;
	decEnv_t decEnv = (decEnv_t)calloc(1,sizeof(struct decEnv_st));
	decEnv->dec25= (decoder_t)calloc(CHNS,sizeof(struct decoder_st));

	for(i=0;i<CHNS;i++)
	{
		decEnv->dec25[i].buf = dec_buf_new(4*1024*1024);
		decEnv->dec25[i].PKG_STARTED = 0;
	}
	return decEnv;
}

void free_dec_chns(decEnv_t decEnv)
{
	int i;
	for(i=0;i<CHNS;i++){
		dec_buf_free(decEnv->dec25[i].buf);
	}
}
