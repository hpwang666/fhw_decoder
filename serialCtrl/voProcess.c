#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>


#include "connet.h"
#include "voProcess.h"
#include "media.h"
#include "env.h"

extern loop_ev ev;
		
int server_read_handle(event_t ev)
{
	int r;
	
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
			
			//printf("recv>>%s\n",buf->head);
			buf_consume(buf, r);
			if(ev->ready){
				printf("ready\n");
				//handle_read_event(ev);//此句注销后，必须用while循环将缓冲区的数据读完，这样好处就是  减少了event触发次数。
			}
		}
	}while(ev->ready);
	
	//handle_read_event(ev);//此句只有在缓存数据没读完才会触发，以上的循环已经把数据读完了
	return 0;
}

int server_write_handle(event_t ev)
{
	int n;
	conn_t c =(conn_t) ev->data;
	n = c->send(c,(u_char *)"send again",10);
	if(!c->write->ready) printf("only send %d bytes\n",n);
	return 0;
}

int init_udp_conn(conn_t c,void *arg)
{
	c->read->handler = server_read_handle;
	c->write->handler = server_write_handle;
	c->data = arg;//lc->ls_arg
	return 0;
}


int rtsp_connect_handler(event_t ev)
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
	
	return 0;
}


int rtsp_reconnect_peer(event_t ev)
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


int transVo(conn_t c, custom_t customCmd)
{
	int i =0;	
	struct netConfig_st netCfg;
	int voMutx = (customCmd->cmd&0xff)>>4;
	int chn = customCmd->ch;

	int sqlite3Chn = voMutx*chn;
	printf("%02x:%d:%d\r\n",customCmd->cmd,voMutx,sqlite3Chn);
	for(i=0;i<voMutx;i++){
		memset((u_char *)&netCfg,0,sizeof(struct netConfig_st));
		netCfg.magic=PKG_MAGIC;
		netCfg.chn=i;
		netCfg.subwin=voMutx;

		sprintf(netCfg.mediaInfo.camAddress,"%s",ev->camConn[i+sqlite3Chn].address);
		if(voMutx==1 || voMutx ==4)
			sprintf(netCfg.mediaInfo.camUrl,"/h264/ch1/main/av_stream");
		else
			sprintf(netCfg.mediaInfo.camUrl,"/h264/ch1/sub/av_stream");
		sprintf(netCfg.mediaInfo.camUser,"admin");
		sprintf(netCfg.mediaInfo.camPasswd,"fhjt12345");
		netCfg.mediaInfo.camPort =554;

		printf("cam ip :%s \r\n",netCfg.mediaInfo.camAddress);
		//memset(netCfg.mediaInfo.camAddress,0,32);
		c->send(c,(u_char *)&netCfg,sizeof(struct netConfig_st));
	}
	return 0;
}
