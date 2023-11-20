/*added in 2020/4/8*/


#include "core.h"
#include "connection.h"
#include "timer.h"
#include "event.h"
#include "media.h"

#undef  _DEBUG
#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif 


extern msec64   current_msec;
static int connect_handler(event_t ev);
static int reconnect_peer(event_t ev);
static int send_handler(event_t ev);

//这两个handle可以放在模块层，由具体应用来实现
int server_read_handle(event_t ev)
{
	int r;
	conn_t c = (conn_t)ev->data;
	buf_t buf=c->readBuf;
	buf_extend(buf, 4096);
	r = c->recv(c,buf->tail,4096);
	if(r<=0){
		if(r==0){
			close_conn(c);
			debug("peer conn closed:%s:%d\n",c->peer_ip,c->peer_port);
		}
		else handle_read_event(ev);
	}
	else{
		buf->size += r;
		buf->tail += r;
		
		printf("%d\n",r);
		add_timer(c->write,12000);		
		buf_consume(buf, r);
		if(ev->ready){
			handle_read_event(ev);
		}
		
	}
	return 0;
}



static int got_sig_term = 0;
static void on_sig_term(int sig)
{
	got_sig_term = 1;
	printf("term\n\r");
}

int main()
{
	msec64 t,delta;
	
	signal(SIGTERM, on_sig_term);
	signal(SIGQUIT, on_sig_term);
	signal(SIGINT, on_sig_term);
	
	init_conn_queue();
	init_timer();
	init_epoll();
	
	int peerfd;
	conn_t pc;
	peerfd = connect_peer("172.16.10.58",10000,&pc);
	if(peerfd == AIO_AGAIN){
		pc->read->handler = connect_handler;
		pc->write->handler = connect_handler;//write超时函数
		add_timer(pc->write, 5000);//加入定时器去判断
	}
	if(peerfd == AIO_OK);
	
	
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
	free_timer();
	free_epoll();
	
	return 0;
}

static int send_handler(event_t ev)
{
	conn_t      c;
    c = ev->data;
	struct netConfig_st netCfg;
	int i;
	static int send_0=1;
#if 0	 
	 
	
		
	i=0;
	
	int sub=4;
	for(i=0;i<sub;i++){
		memset((u_char *)&netCfg,0,sizeof(struct netConfig_st));
		netCfg.magic=PKG_MAGIC;
		netCfg.chn=i;
		netCfg.subwin=sub;
		sprintf(netCfg.mediaInfo.camAddress,"172.16.10.44");
		sprintf(netCfg.mediaInfo.camUrl,"/h264/ch%d/main/av_stream",1);
		sprintf(netCfg.mediaInfo.camUser,"admin");
		sprintf(netCfg.mediaInfo.camPasswd,"fhjt12345");
		netCfg.mediaInfo.camPort =554;
		
		//memset(netCfg.mediaInfo.camAddress,0,32);
		c->send(c,(u_char *)&netCfg,sizeof(struct netConfig_st));
	}
	
	
	
	
	
#else	
	if(send_0)	{
		

		for(i=0;i<2;i++){
			memset((u_char *)&netCfg,0,sizeof(struct netConfig_st));
			netCfg.magic=PKG_MAGIC;
			netCfg.chn=i;
			netCfg.subwin=2;
			sprintf(netCfg.mediaInfo.camAddress,"172.16.10.%d",44);
			sprintf(netCfg.mediaInfo.camUrl,"/h264/ch%d/main/av_stream",1);
			sprintf(netCfg.mediaInfo.camUser,"admin");
			sprintf(netCfg.mediaInfo.camPasswd,"fhjt12345");
			netCfg.mediaInfo.camPort =554;
			
			c->send(c,(u_char *)&netCfg,sizeof(struct netConfig_st));
		}
		
	}
	else{
		
		for(i=0;i<4;i++){
			memset((u_char *)&netCfg,0,sizeof(struct netConfig_st));
			netCfg.magic=PKG_MAGIC;
			netCfg.chn=i;
			netCfg.subwin=9;
			sprintf(netCfg.mediaInfo.camAddress,"172.16.10.%d",44);
			sprintf(netCfg.mediaInfo.camUrl,"/h264/ch1/sub/av_stream");
			sprintf(netCfg.mediaInfo.camUser,"admin");
			sprintf(netCfg.mediaInfo.camPasswd,"fhjt12345");
			netCfg.mediaInfo.camPort =554;
			
			c->send(c,(u_char *)&netCfg,sizeof(struct netConfig_st));
		}
		for(i=4;i<7;i++){
			memset((u_char *)&netCfg,0,sizeof(struct netConfig_st));
			netCfg.magic=PKG_MAGIC;
			netCfg.chn=i;
			netCfg.subwin=9;
			sprintf(netCfg.mediaInfo.camAddress,"172.16.10.%d",43);
			sprintf(netCfg.mediaInfo.camUrl,"/h264/ch1/sub/av_stream");
			sprintf(netCfg.mediaInfo.camUser,"admin");
			sprintf(netCfg.mediaInfo.camPasswd,"fhjt12345");
			netCfg.mediaInfo.camPort =554;
			
			c->send(c,(u_char *)&netCfg,sizeof(struct netConfig_st));
		}
		for(i=7;i<9;i++){
			memset((u_char *)&netCfg,0,sizeof(struct netConfig_st));
			netCfg.magic=PKG_MAGIC;
			netCfg.chn=i;
			netCfg.subwin=9;
			sprintf(netCfg.mediaInfo.camAddress,"172.16.10.%d",42);
			sprintf(netCfg.mediaInfo.camUrl,"/h264/ch1/sub/av_stream");
			sprintf(netCfg.mediaInfo.camUser,"admin");
			sprintf(netCfg.mediaInfo.camPasswd,"fhjt12345");
			netCfg.mediaInfo.camPort =554;
			
			c->send(c,(u_char *)&netCfg,sizeof(struct netConfig_st));
		}
		
	}
	send_0 ^=1;
#endif	
	return 0;
}


static int connect_handler(event_t ev)
{
    conn_t      c;
    c = ev->data;
	
    if (ev->timedout) {
        printf("CONN:conn timeout\r\n");
		ev->timedout = 0;
		if(AIO_AGAIN == reconnect_peer(ev)){
			return AIO_AGAIN;
		}
    }
	
	//此处只是返回错误不再进行重连操作
    if (test_connect(c) != AIO_OK) {//handled  by wev->handler
        printf("CONN:conn failed\r\n");
        return AIO_ERR;
    }
	
	del_timer(c->write);
	set_conn_info(c);
	printf("Connected,client=%s:%d peer=%s:%d\n", c->local_ip,c->local_port,c->peer_ip,c->peer_port);
	
	c->write->ready = 1;//可写
	add_event(c->read,READ_EVENT);
	c->read->handler =server_read_handle;
	c->write->handler =send_handler;
	add_timer(c->write,10);
	return 0;
}

static int reconnect_peer(event_t ev)
{
	struct sockaddr_in sa;
	int rc;
	conn_t      c;
	c = ev->data;
	
	if (c->read->timer_set)  del_timer(c->read); 
    if (c->write->timer_set) del_timer(c->write);       
	if (c->read->active) 	del_event(c->read,0,CLOSE_EVENT);	
	if (c->write->active ) 	del_event(c->write,0,CLOSE_EVENT);
	if(-1 != c->fd)	 {close(c->fd); c->fd =-1;} //注意，close fd 之后，所有的epoll  event 都失效了
	
	
	sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(c->peer_ip);	
    sa.sin_port = htons(c->peer_port);			
	
    if((c->fd = socket(AF_INET,SOCK_STREAM,0)) < 0) return -1;

	nonblocking(c->fd);
	keepalive(c->fd);
	
    rc = connect(c->fd,(struct sockaddr*)&sa, sizeof(struct sockaddr_in));
	
    if(rc == 0)
    {
        debug("RECONN:already ok?");
		add_event(c->read, READ_EVENT);
		return AIO_OK;
    }
	if(rc == -1 && (CONN_WOULDBLOCK || CONN_INPROGRESS))//非阻塞都会执行到这一步
    {
        debug("RECONN:need check\r\n");
		c->write->handler = connect_handler;
		add_event(c->write, WRITE_EVENT);
		add_timer(c->write, 3000);
		return AIO_AGAIN;
    }
	//close(fd);
	close_conn(c);
	return AIO_ERR;
}


