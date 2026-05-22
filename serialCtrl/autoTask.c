#include <stdlib.h>
#include <stdio.h>

#include "env.h"
#include "autoTask.h"
#include "osal.h"

extern loop_ev	env;

#undef LOG_HANDLE
#define LOG_HANDLE
#ifdef  LOG_HANDLE
	#define log_debug(...) zlog_debug(env->zc,__VA_ARGS__)
	#define log_info(...) zlog_info(env->zc,__VA_ARGS__)
	#define log_err(...)  zlog_error(env->zc,__VA_ARGS__)
#else
	#define log_debug(...) printf(__VA_ARGS__);printf("\r\n")
	#define log_info(...) printf(__VA_ARGS__);printf("\r\n")
	#define log_err(...) printf(__VA_ARGS__);printf("\r\n")
#endif

int osal_auto_vo(int task_id,int events)
{
	int do_event = events;
	
	if( events & OSALSTART){
		printf("****osal start****\r\n");
		do_event ^= OSALSTART;
	}
	if ( events & OSAL_VO )
	{
		log_debug("do auto vo");
        queue_push(env->voQueue,1,sizeof(struct custom_st),&(env->serialCustom));
        do_event ^= OSAL_VO;
	}
	
	return do_event;
}