#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>


#include "connet.h"
#include "voProcess.h"
#include "media.h"
#include "env.h"

		

#undef LOG_HANDLE
//#define LOG_HANDLE
#ifdef  LOG_HANDLE
	#define log_info(...) zlog_info(env->zc,__VA_ARGS__)
	#define log_err(...)  zlog_error(env->zc,__VA_ARGS__)
#else
	#define log_info(...) printf(__VA_ARGS__);printf("\r\n")
	#define log_err(...) printf(__VA_ARGS__);printf("\r\n")
#endif

int rtsp_read_handle(event_t ev)
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

int rtsp_write_handle(event_t ev)
{
	int n;
	conn_t c =(conn_t) ev->data;
	n = c->send(c,(u_char *)"send again",10);
	if(!c->write->ready) printf("only send %d bytes\n",n);
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
	c->read->handler =rtsp_read_handle;
	c->write->handler = rtsp_write_handle;
	
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


int transVo(loop_ev env,conn_t c, custom_t customCmd)
{
	int i =0;	
	struct netConfig_st netCfg;
	int voMutx ;
	int chn ;
	char cams[64];
	int chns[16];
	const char* sep = ",";
	int allChns=0;
	char* p = NULL;
	pool_t pool;

	if(customCmd->cmd != 0xaa){//串口切屏
		
		voMutx = (customCmd->cmd&0xff)>>4;
		chn = customCmd->ch;
		if(voMutx == 15) voMutx =16;//把0x0f--15 变成16
		if(voMutx==8) voMutx=6;//暂时不支持8画面
		int sqlite3Chn = voMutx*chn;
		//这里需要对9-3  里面超过的部分进行限制
		allChns = voMutx;
		if(sqlite3Chn==27) allChns = 5;//9-3
		log_info("cmd:%02x vo:%d sqlite3:%d",customCmd->cmd,voMutx,sqlite3Chn);
		for(i=0;i<voMutx;i++){
			memset((u_char *)&netCfg,0,sizeof(struct netConfig_st));
			netCfg.magic=PKG_MAGIC;
			netCfg.chn=i;
			netCfg.subwin=voMutx;

			if(i>=allChns){
				netCfg.mediaInfo.camAddress[0] = 0;//这个小格格就是黑屏
			}
			else{
				if(strncmp(env->camConn[i+sqlite3Chn].address,"0.0.0.0",7)==0)
					netCfg.mediaInfo.camAddress[0] = 0;//这个小格格就是黑屏
				else
					sprintf(netCfg.mediaInfo.camAddress,"%s",env->camConn[i+sqlite3Chn].address);


				sprintf(netCfg.mediaInfo.camUrl,"%s",env->camConn[i+sqlite3Chn].url);
				pool = get_pool(env->poolList,4096);
				p=netCfg.mediaInfo.camUrl;
				if(env->decType)
					p=str_replace(pool,p,"264","265");
				if(voMutx==1){
					;
				}
				else if(voMutx==4){
					if(env->muxt4==0) p=str_replace(pool,p,"main","sub");
				}
				else
					p=str_replace(pool,p,"main","sub");

				//针对没有子码流的摄像头
				//if(i==0&&voMutx>1){
				//	if(str_nstr((u_char*)p,"sub",strlen(p)))
				//		p=str_replace(pool,p,"sub","main");
				//}
				if(i==0&&voMutx==6){
					if(str_nstr((u_char*)p,"sub",strlen(p)))
						p=str_replace(pool,p,"sub","main");
				}


				sprintf(netCfg.mediaInfo.camUrl,"%s",p);
				destroy_pool(pool);
			}
			sprintf(netCfg.mediaInfo.camUser,"admin");
			sprintf(netCfg.mediaInfo.camPasswd,env->passwd);
			netCfg.mediaInfo.camPort =554;

			log_info("cam :%s %s",netCfg.mediaInfo.camAddress,netCfg.mediaInfo.camUrl);
			//memset(netCfg.mediaInfo.camAddress,0,32);

			c->send(c,(u_char *)&netCfg,sizeof(struct netConfig_st));
		}
	}
	if(customCmd->cmd == 0xaa){//左右联动切屏
		chn = customCmd->stop;
		sprintf(cams,"%s",env->plcCams[chn-1]);

		i=0;
		for (p = strtok(cams, sep);p != NULL;p=strtok(NULL,sep)) {
			if(atoi(p)>0 && atoi(p)<33)
				chns[i]=atoi(p);
			else chns[i] = 1;
			i++;
		}
		allChns=i;
		printf("plc_chn:%d\r\n",i);
		switch (i) {
			case 1 :voMutx=1;break;
			case 2 :voMutx=2;break;
			case 3:
			case 4 :voMutx=4;break;
			case 5 :
			case 6 :voMutx=6;break;
			case 7 :
			case 8 :
			case 9 :
					voMutx=9;break;
			case 10 :
			case 11 :
			case 12 :
			case 13 :
			case 14 :
			case 15 :
			case 16 :
					voMutx=16;break;
			default:voMutx=1;break;
		}
		for(i=0;i<allChns;i++){
			memset((u_char *)&netCfg,0,sizeof(struct netConfig_st));
			netCfg.magic=PKG_MAGIC;
			netCfg.chn=i;
			netCfg.subwin=voMutx;

			if(strncmp(env->camConn[chns[i]-1].address,"0.0.0.0",7)==0)
				netCfg.mediaInfo.camAddress[0] = 0;//这个小格格就是黑屏
			else
				sprintf(netCfg.mediaInfo.camAddress,"%s",env->camConn[chns[i]-1].address);
			sprintf(netCfg.mediaInfo.camUrl,"%s",env->camConn[chns[i]-1].url);
			pool = get_pool(env->poolList,4096);

			p=netCfg.mediaInfo.camUrl;
			if(env->decType)
				p=str_replace(pool,p,"264","265");
			if(voMutx==1){
				;
			}
			else if(voMutx==4){
				if(env->muxt4==0) p=str_replace(pool,p,"main","sub");
			}
			else
				p=str_replace(pool,p,"main","sub");

			//针对没有子码流的摄像头
			//if(i==0&&voMutx>1){
			//	if(str_nstr((u_char*)p,"sub",strlen(p)))
			//		p=str_replace(pool,p,"sub","main");
			//}
			sprintf(netCfg.mediaInfo.camUrl,"%s",p);
			destroy_pool(pool);

			sprintf(netCfg.mediaInfo.camUser,"admin");
			sprintf(netCfg.mediaInfo.camPasswd,env->passwd);
			netCfg.mediaInfo.camPort =554;

			printf("cam :%s %s \r\n",netCfg.mediaInfo.camAddress,netCfg.mediaInfo.camUrl);
			c->send(c,(u_char *)&netCfg,sizeof(struct netConfig_st));
		}
	}
	return 0;
}
