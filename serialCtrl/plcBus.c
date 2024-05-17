#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "env.h"

#include "connet.h"
static int plc_connect_handler(event_t ev);
static int plc_reconnect_peer(event_t ev);
uint16_t check_crc(uint8_t* pbuffer, int length);
		
int plc_read_handle(event_t ev)
{
	int r;
	int cmd;	
	struct custom_st custom;
	static int zoomCmd=-1;
	int tempHeight;
	uint16_t height_H;
	static int voCmd=-1;
	conn_t c = (conn_t)ev->data;
	loop_ev env = (loop_ev)c->data;
	if (ev->timedout) {//数据读取超时
		//close_conn(c);//此处不能close，会释放conn，引起错误指针
		ev->timedout = 0;
		//plc_reconnect_peer(ev);
		return AIO_ERR;
	}
	//del_timer(c->read);
	buf_t buf=c->readBuf;
	do{
		buf_extend(buf, 4096);
		r = c->recv(c,buf->tail,4096);
		if(r<=0){
			if(r==0){
				plc_reconnect_peer(ev);
				printf("peer conn closed:%s:%d\n",c->peer_ip,c->peer_port);
			}
			else handle_read_event(ev);
		}
		else{
			buf->size += r;
			buf->tail += r;
			
			printf("recv>>%d\n",r);

			//00 03 06 00 00 00 00 01 4d 22 33
			if(r==11){
				if( 0/*buf->head[6]^voCmd*/){
					voCmd=buf->head[6];
					if(voCmd==0) cmd =3;
					if(voCmd==0x04) cmd =1;
					if(voCmd==0x02) cmd =2;
					if(cmd>0 && cmd<4){
						custom.ch =0; 
						custom.cmd = 0xaa;
						custom.stop =cmd;//0x01--LEFT 0X02--RIGHT 0X03--STOP 
						queue_push(env->voQueue,1,sizeof(struct custom_st),&custom);
					}
				}
				if(buf->head[3]&0x80){//负数，在地平面之下  3300是地上的高度
					height_H=(buf->head[3]<<8)&0xff00;
					tempHeight = 3300+(0xffff-height_H-buf->head[4]);
				}
				else{
					height_H=(buf->head[3]<<8)&0xff00;
					tempHeight = 3300-(height_H+buf->head[4]);
				}
				printf("height0:%d\r\n",tempHeight);
				tempHeight=(tempHeight/99)&0xff;
				printf("height1:%d\r\n",tempHeight);
				if( tempHeight^zoomCmd){
					zoomCmd=tempHeight;
					cmd = tempHeight;
					custom.ch = 0;	
					custom.cmd = 0xaa;
					custom.stop =cmd; 
					queue_push(env->ptzQueue,1,sizeof(struct custom_st),&custom);

					//把画面切换到1通道
					custom.ch =0; 
					custom.cmd = 0x1a;
					custom.stop =0;//这里不需要 
					queue_push(env->voQueue,1,sizeof(struct custom_st),&custom);
				}
			}

			buf_consume(buf, r);
			if(ev->ready){
				printf("ready\n");
			}
		}
	}while(ev->ready);
	add_timer(c->write,2000);

	//handle_read_event(ev);//此句只有在缓存数据没读完才会触发，以上的循环已经把数据读完了
	return 0;
}

uint16_t check_crc(uint8_t* pbuffer, int length)
{
	uint16_t wcrc = 0xffff;
	int i,j;
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


int plc_write_handle(event_t ev)
{
	int n;
	conn_t c =(conn_t) ev->data;
	loop_ev env = (loop_ev)c->data;
	u_char plcBus[32];
	plcBus[0]=0x04;
	plcBus[1]=0x03;
	plcBus[2]=(env->r0>>8)&0xff;
	plcBus[3]=(env->r0)&0xff;
	plcBus[4]=0x00;
	plcBus[5]=0x03;//读取多少个字
	*(uint16_t*)(plcBus+ 6) = check_crc(plcBus, 6);
	//printf("I have sent 8 bytes\r\n");
	n = c->send(c,plcBus,8);
	if(!c->write->ready) printf("only send %d bytes\n",n);
	add_timer(c->read,10000);
	return 0;
}



int initPlcBus(loop_ev env)
{
	conn_t  pc;
	int ret;
	

	printf("%s:%d\r\n",env->plcAddr,env->plcPort);
	ret = connect_peer(env->plcAddr,env->plcPort,&pc);
	pc->data = env;//传入参数
	if(ret == AIO_AGAIN){
		pc->read->handler = plc_connect_handler;
		pc->write->handler = plc_connect_handler;//write超时函数
		add_timer(pc->write, 1000);//加入定时器去判断
	}
	if(ret == AIO_OK);
	
	return 0;
}

static int plc_connect_handler(event_t ev)
{
    conn_t      c;
    //ngx_stream_session_t  *s;

    c = ev->data;
    //plcClient_t rc = c->data;

    if (ev->timedout) {
        printf("CONN:conn timeout\r\n");
		ev->timedout = 0;
		if(AIO_AGAIN == plc_reconnect_peer(ev)){
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
	c->read->handler =plc_read_handle;
	c->write->handler = plc_write_handle;
	add_timer(c->write, 1000);//加入定时器去判断
	
	return 0;
}


static int plc_reconnect_peer(event_t ev)
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
		c->write->handler = plc_connect_handler;
		add_event(c->write, WRITE_EVENT);
		add_timer(c->write, 5000);
		return AIO_AGAIN;
    }
	
	//free_plc_clients(c->data);
	return AIO_ERR;
}
