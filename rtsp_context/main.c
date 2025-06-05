/*added in 2020/4/8*/


#include "core.h"
#include "connection.h"
#include "timer.h"
#include "event.h"
#include "rtsp.h"



#undef  _DEBUG
#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif 


extern msec64   current_msec;
pooList_t list;



//这两个handle可以放在模块层，由具体应用来实现


static int got_sig_term = 0;
static void on_sig_term(int sig)
{
	got_sig_term = 1;
	printf("term\n\r");
}

int main()
{
	msec64 t,delta;
	
	conn_t lc;
	rtspClient_t rc[16];
	
	signal(SIGTERM, on_sig_term);
	signal(SIGQUIT, on_sig_term);
	signal(SIGINT, on_sig_term);
	
	init_conn_queue();
	init_timer();
	init_epoll();
	list = create_pool_list();

	
#if 0
	rc[1] = init_rtsp_clients(list,"10.14.5.17",554,"admin","ABCabc123","/h264/ch1/main/av_stream");	
	rc[1]->chn = 1;
#else 
	rc[1] = init_rtsp_clients(list,"192.168.1.44",554,"admin","fhjt12345","/h264/ch1/main/av_stream");
	rc[1]->chn = 1;

#endif 
	while(!got_sig_term)
	{
		t = find_timer();
		process_events(t,1);
		if(current_msec -delta) {
			expire_timers();
			delta = current_msec;
		}
	}
	
	free_all_conn();
	free_pool_list(list);
	free_timer();
	free_epoll();
	
	return 0;
}





