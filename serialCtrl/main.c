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
#include "plcBus.h" 

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
	loop_ev ev = (loop_ev)arg;
	char data[4];//we define offset=0 channel,
				//offset=1 cmd
				//offset=2 start/stop
	struct custom_st custom;
	while(ev->uart_running)
	{
		if(4==serialRecv(ev->serialFd,data)){
			switch (data[1]){
				case 0x1a:
				case 0x2a:
				case 0x4a:
				case 0x6a:
				case 0x9a:
				case 0xfa:
					ev->voType = data[1]	;//存储是为了判断单画面下操作PTZ
					ev->channel = data[2]-0x80;//多画面下4-1 4-2 ...
					custom.ch = ev->channel;
					custom.cmd = ev->voType;
					custom.stop = data[3];//在这里没有用
					queue_push(ev->voQueue,1,sizeof(struct custom_st),&custom);
					break;
				case 0x0a:
					custom.ch = ev->channel;
					if(data[2]==0){
						custom.cmd = ev->lastCmd;	
						custom.stop = 1;
					}
					else{
						custom.cmd = data[2]*4+data[3]-0x80;
						custom.stop = 0;
						ev->lastCmd = custom.cmd;
					}
					if(ev->voType ==0x1a)//必须是单画面下
						queue_push(ev->ptzQueue,0,sizeof(struct custom_st),&custom);
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

	loop_ev ev = (loop_ev)arg;

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
	init_udp_conn(lc,ev);



	if(ev->protocol){
		initPlcBus(ev);
	}
	while(ev->vo_running)
	{
		t = find_timer();
		process_events(t,1);
		if(get_current_ms() -delta) {
			expire_timers();
			delta = get_current_ms();
		}


		tmp = queue_get(ev->voQueue);
		if(tmp){
			transVo(ev,decConn,(custom_t)tmp->data);
			queue_cache(ev->voQueue,tmp);
		} 
	
	}


	free_all_conn();
	free_timer();
	free_epoll();
	return 0;
}

void *_httpLoop(void  *arg)
{
	loop_ev ev = (loop_ev)arg;
	queueNode_t tmp;
	while(ev->http_running)
	{
		usleep(30000);
		tmp = queue_get(ev->ptzQueue);
		if(tmp){
			transCmd(ev,(custom_t)tmp->data);
			queue_cache(ev->ptzQueue,tmp);
		} 
	}
	return 0;
}


loop_ev init (void)
{
	int i =0;
	loop_ev ev;
	struct custom_st custom;
	ev = (loop_ev)calloc(1,sizeof(struct _loop_ev));
	ev->camConn = (camConnection)calloc(16,sizeof(struct _camConnection));
	for(i=0;i<16;i++)
	{
		ev->camConn[i].channel = i;
	}
	getChnnelInfo(ev);
	
	ev->serialFd = initSerial();
	ev->uart_running = 1;
	ev->vo_running =1 ;
	ev->http_running = 1;
	
	ev->ptzQueue=queue_new(16,sizeof(struct custom_st ));
	ev->voQueue=queue_new(16,sizeof(struct custom_st ));
	
	custom.ch = 0;
	custom.cmd = 0x1a;
	custom.stop = 0;//在这里没有用
	queue_push(ev->voQueue,1,sizeof(struct custom_st),&custom);
	pthread_create(&ev->uart_worker, NULL, _mainLoop, ev);
	pthread_create(&ev->vo_worker, NULL, _voLoop, ev);
	pthread_create(&ev->http_worker, NULL, _httpLoop, ev);
	
	pthread_mutex_init(&ev->mutex, NULL);
	pthread_mutex_init(&ev->mutex_cmd, NULL);
	return ev;
}

void clean(loop_ev ev)
{
	pthread_mutex_destroy(&ev->mutex);
	closeSerial(ev->serialFd);
	free(ev->camConn);
	queue_free(ev->ptzQueue);
	queue_free(ev->voQueue);
	free(ev);
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
