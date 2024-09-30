#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>


#include "connet.h"
static int rtsp_connect_handler(event_t ev);
static int rtsp_reconnect_peer(event_t ev);
int packread(uint8_t* pbuffer, int iaddress, int ibegin, int icount);
uint16_t check_crc(uint8_t* pbuffer, int length);
static int got_sig_term = 0;
static void on_sig_term(int sig)
{
	got_sig_term = 1;
	printf("term......\n\r");
}

		
int server_read_handle(event_t ev)
{
	int r;
	int i=0;	
	conn_t c = (conn_t)ev->data;
	//threadPool_t tp = (threadPool_t)c->data;//lc->ls_arg
	buf_t buf=c->readBuf;
	do{
		buf_extend(buf, 4096);
		r = c->recv(c,buf->tail,4096);
		if(r<=0){
			if(r==0){
				close_conn(c);
				printf("peer conn closed:%s:%d\n",c->peer_ip,c->peer_port);
			}
			else handle_read_event(ev);
		}
		else{
			buf->size += r;
			buf->tail += r;
			
			for(i=0;i<r;i++)
				printf("%02x ",*(buf->head+i));
			printf("\r\n");
			buf_consume(buf, r);
			add_timer(c->write, 2000);
			if(ev->ready){
				printf("ready\n");
				//handle_read_event(ev);//此句注销后，必须用while循环将缓冲区的数据读完，这样好处就是  减少了event触发次数。
			}
		}
	}while(ev->ready);
	
	//handle_read_event(ev);//此句只有在缓存数据没读完才会触发，以上的循环已经把数据读完了
	return 0;
}

uint16_t check_crc(uint8_t* pbuffer, int length)
{
	uint16_t wcrc = 0xffff;
	int j=0;
	int i=0;
	for (i = 0; i < length; i++)
	{
		wcrc ^= pbuffer[i];

		for (j = 0; j < 8; j++)
			if (wcrc & 0x0001)
				wcrc = (wcrc >> 1) ^ 0xa001;
			else
				wcrc = wcrc >> 1;
	}
	return wcrc;
}

int packread(uint8_t* pbuffer, int iaddress, int ibegin, int icount)
{
	int i;
	pbuffer[0] = iaddress;
	pbuffer[1] = 0x03;
	pbuffer[2] = (ibegin>>8)&0xff;
	pbuffer[3] = (ibegin)&0xff;
	pbuffer[4] = (icount>>8)&0xff;
	pbuffer[5] = (icount)&0xff;
	*(uint16_t*)(pbuffer + 6) = check_crc(pbuffer, 6);
	printf("send: ");
	for(i=0;i<8;i++)
		printf("%02x ",(pbuffer[i]));
	printf("\r\n");

	return 8;
}
int server_write_handle(event_t ev)
{
	int n;
	int res;
	uint8_t m[16];
	conn_t c =(conn_t) ev->data;
    if (ev->timedout) {
        printf("try send m\r\n");
		ev->timedout = 0;
    }
	res =packread( m, 0,2800, 1);
	n = c->send(c,(u_char *)m,res);
	if(!c->write->ready) printf("only send %d bytes\n",n);
	return 0;
}


int init_accepted_conn(conn_t c,void *arg)
{
	c->read->handler = server_read_handle;
	c->write->handler = server_write_handle;
	c->data = arg;//lc->ls_arg
	printf("Acceped,client=%s:%d peer=%s:%d\n", c->local_ip,c->local_port,c->peer_ip,c->peer_port);
	add_event(c->read,READ_EVENT);
	
	return 0;
}

int main()
{
	int ret;
	conn_t pc;
	msec64 t,delta=0;
	
	signal(SIGTERM, on_sig_term);
	signal(SIGQUIT, on_sig_term);
	signal(SIGINT, on_sig_term);
	
	
	init_conn_queue();
	init_timer();
	init_epoll();

	
	

	ret = connect_peer("192.168.1.11",10090,&pc);
	pc->data = NULL;//传入参数
	if(ret == AIO_AGAIN){
		pc->read->handler = rtsp_connect_handler;
		pc->write->handler = rtsp_connect_handler;//write超时函数
		add_timer(pc->write, 1000);//加入定时器去判断
	}
	if(ret == AIO_OK);
	
	
	
	while(!got_sig_term)
	{
		t = find_timer();
		process_events(t,1);
		if(get_current_ms() -delta) {
			expire_timers();
			delta = get_current_ms();
		}
	}
	
	free_all_conn();
	free_timer();
	free_epoll();
	
	return 0;
}

static int rtsp_connect_handler(event_t ev)
{
    conn_t      c;
    //ngx_stream_session_t  *s;

    c = ev->data;
    //rtspClient_t rc = c->data;

    if (ev->timedout) {
        printf("CONN:conn timeout\r\n");
		ev->timedout = 0;
		if(AIO_AGAIN == rtsp_reconnect_peer(ev)){
			return AIO_AGAIN;
		}
    }
	
    if (test_connect(c) != AIO_OK) {//handled  by wev->handler
        printf("TEST_CONN:conn failed\r\n");
        return AIO_ERR;
    }
	
	if( -1  == set_conn_info(c)) {
		return AIO_ERR;
	}
	del_timer(c->write);
	printf("Connected,client=%s:%d peer=%s:%d\n", c->local_ip,c->local_port,c->peer_ip,c->peer_port);
	
	c->write->ready = 1;//可写
	add_event(c->read,READ_EVENT);
	c->read->handler =server_read_handle;
	c->write->handler = server_write_handle;
	add_timer(c->write, 2000);
	return 0;
}


static int rtsp_reconnect_peer(event_t ev)
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
	buf_init(c->readBuf);
	
	sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(c->peer_ip);	
    sa.sin_port = htons(c->peer_port);			
	
    if((c->fd = socket(AF_INET,SOCK_STREAM,0)) < 0) return -1;

	nonblocking(c->fd);
	keepalive(c->fd);
	
    rc = connect(c->fd,(struct sockaddr*)&sa, sizeof(struct sockaddr_in));
	
    if(rc == 0)
    {
        printf("RECONN:already ok?");
		add_event(c->read, READ_EVENT);
		return AIO_OK;
    }
	
	if(rc == -1 && (CONN_WOULDBLOCK || CONN_INPROGRESS))//非阻塞都会执行到这一步
    {
        printf("RECONN:need check\r\n");
		c->write->handler = rtsp_connect_handler;
		add_event(c->write, WRITE_EVENT);
		add_timer(c->write, 5000);
		return AIO_AGAIN;
    }
	
	//free_rtsp_clients(c->data);
	return AIO_ERR;
}
