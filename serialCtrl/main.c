#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "queue.h"
#include <env.h>
#include "hikisapi.h"
#include "httpclient.h"

#include <uart.h>
#include <ptz.h>
#include "connet.h"
#include "voProcess.h"
#include "udpServer.h"
 

#undef  _DEBUG
#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif
loop_ev env;

static void on_sig_term(int sig);

void *_mainLoop(void  *arg)  
{  
	loop_ev env = (loop_ev)arg;
	char data[4];//we define offset=0 channel,
				//offset=1 cmd
				//offset=2 start/stop
	struct custom_st custom;
	while(env->uart_running)
	{
		if(4==serialRecv(env->serialFd,data)){
			switch (data[1]){
				case 0x1a:
				case 0x2a:
				case 0x4a:
				case 0x6a:
				case 0x9a:
				case 0xfa:
					env->voType = data[1]	;//存储是为了判断单画面下操作PTZ
					env->channel = data[2]-0x80;//多画面下4-1 4-2 ...
					custom.ch = env->channel;
					custom.cmd = env->voType;
					custom.stop = data[3];//在这里没有用
					queue_push(env->voQueue,1,sizeof(struct custom_st),&custom);
					break;
				case 0x0a:
					custom.ch = env->channel;
					if(data[2]==0){
						custom.cmd = env->lastCmd;	
						custom.stop = 1;
					}
					else{
						custom.cmd = data[2]*4+data[3]-0x80;
						custom.stop = 0;
						env->lastCmd = custom.cmd;
					}
					if(env->voType ==0x1a)//必须是单画面下
						queue_push(env->ptzQueue,0,sizeof(struct custom_st),&custom);
					break;
				default:
					printf("get invalid data %02x\r\n",data[1]);
					break;
			}
		}	
	}
	return 0;
}  

void *_voLoop(void  *arg)
{
	int ret;
	conn_t lc ,decConn;
	msec64 t,delta=0;
	queueNode_t tmp;

	loop_ev env = (loop_ev)arg;

	printf("vo loop start...\r\n");
	init_conn_queue();
	init_timer();
	init_epoll();


	ret = connect_peer("127.0.0.1",10000,&decConn);
	decConn->data = NULL;//传入参数
	if(ret == AIO_AGAIN){
		decConn->read->handler = rtsp_connect_handler;
		decConn->write->handler = rtsp_connect_handler;//write超时函数
		add_timer(decConn->write, 1000);//加入定时器去判断
	}
	if(ret == AIO_OK);


	lc = create_listening_udp(11000);
	init_udp_conn(lc,decConn);
	while(env->vo_running)
	{
		t = find_timer();
		process_events(t,1);
		if(get_current_ms() -delta) {
			expire_timers();
			delta = get_current_ms();
		}


		tmp = queue_get(env->voQueue);
		if(tmp){
			transVo(decConn,(custom_t)tmp->data);
			queue_cache(env->voQueue,tmp);
		} 
	
	}


	free_all_conn();
	free_timer();
	free_epoll();
	return 0;
}

void *_httpLoop(void  *arg)
{
	loop_ev env = (loop_ev)arg;
	queueNode_t tmp;
	while(env->http_running)
	{
		usleep(30000);
		tmp = queue_get(env->ptzQueue);
		if(tmp){
			transCmd(env,(custom_t)tmp->data);
			queue_cache(env->ptzQueue,tmp);
		} 
	}
	return 0;
}


loop_ev init (void)
{
	int i =0;
	loop_ev env;
	env = (loop_ev)calloc(1,sizeof(struct _loop_ev));
	env->camConn = (camConnection)calloc(16,sizeof(struct _camConnection));
	for(i=0;i<16;i++)
	{
		env->camConn[i].channel = i;
	}
	getChnnelInfo(env);
	
	env->serialFd = initSerial();
	env->uart_running = 1;
	env->vo_running =1 ;
	env->http_running = 1;
	
	env->ptzQueue=queue_new(16,sizeof(struct custom_st ));
	env->voQueue=queue_new(16,sizeof(struct custom_st ));
	
	pthread_create(&env->uart_worker, NULL, _mainLoop, env);
	pthread_create(&env->vo_worker, NULL, _voLoop, env);
	pthread_create(&env->http_worker, NULL, _httpLoop, env);
	
	pthread_mutex_init(&env->mutex, NULL);
	pthread_mutex_init(&env->mutex_cmd, NULL);
	return env;
}

void clean(loop_ev env)
{
	pthread_mutex_destroy(&env->mutex);
	closeSerial(env->serialFd);
	free(env->camConn);
	queue_free(env->ptzQueue);
	queue_free(env->voQueue);
	free(env);
}
static void on_sig_term(int sig)
{
	env->uart_running = 0;
	env->vo_running =0 ;
	env->http_running =0;
	printf("term......\n\r");
}

int main()
{
	
	env = init();
	signal(SIGTERM, on_sig_term);
	signal(SIGQUIT, on_sig_term);
	signal(SIGINT, on_sig_term);
	pthread_join(env->uart_worker, NULL);
	pthread_join(env->http_worker, NULL);
	pthread_join(env->vo_worker, NULL);
	clean(env); 
	
	return 0;
}
