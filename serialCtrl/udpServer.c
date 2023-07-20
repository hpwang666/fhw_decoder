#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "media.h"

 
#include "env.h"
#include "connet.h"
#include "udpServer.h"

int init_udp_conn(conn_t c,void *arg);

static int check_plc_ctrl_available(u_char *src);
int transVoByPLC(conn_t c, int cmd);

extern loop_ev env;
		
int udp_read_handle(event_t ev)
{
	int r;
	struct custom_st custom;
	conn_t c = (conn_t)ev->data;
	buf_t buf=c->readBuf;
	
	do{
		buf_extend(buf, 4096);
		r = c->recv(c,buf->tail,4096);
		if(r<=0){
			printf("没有数据:%s:%d\n",c->peer_ip,c->peer_port);
			break;
		}
		else{
			buf->size += r;
			buf->tail += r;
			
			if(r==8&& check_plc_ctrl_available(buf->head)){
				printf("%s:%d recv>>%s\n",c->peer_ip,c->peer_port,buf->head);
				if(buf->head[5]==0x32){//切屏
						transVoByPLC(c, buf->head[7]-0x30);
				}
				if(buf->head[5]==0x31){
					custom.ch=0;
					custom.cmd=0xaa;
					custom.stop =((buf->head[6]-0x30)*10+(buf->head[7]-0x30))/4*100;
					queue_push(env->ptzQueue,0,sizeof(struct custom_st),&custom);
				}
			}
			buf_consume(buf, r);
		}
	}while(ev->ready);
	return 0;
}

static int check_plc_ctrl_available(u_char *src)
{
	int i =0 ;
	if(src[0]!='$'||src[1]!='$')
		return 0;
	for(i=2;i<8;i++)
	{
		if(src[i]>0x39||src[i]<0x30)
			return 0;
	}
	return 1;
}

int udp_write_handle(event_t ev)
{
	int n;
	conn_t c =(conn_t) ev->data;
	n = c->send(c,(u_char *)"send again",10);
	if(!c->write->ready) printf("only send %d bytes\n",n);
	return 0;
}

int init_udp_conn(conn_t c,void *arg)
{
	c->read->handler = udp_read_handle;
	c->write->handler = udp_write_handle;
	c->data = arg;//lc->ls_arg
	return 0;
}


int transVoByPLC(conn_t c, int cmd)
{
	int i =0;	
	struct netConfig_st netCfg;
	int voMutx = 4;
	conn_t rtspConn = (conn_t)c->data;
	char cams[32];
	int chns[16];
	const char* sep = ",";
	static int lastCmd=-1;


	if(cmd ^lastCmd)
		lastCmd =cmd;
	else return 0;
	sprintf(cams,"%s",env->plcCams[cmd-1]);


	char* p = NULL;
 
	i=0;
	for (p = strtok(cams, sep);p != NULL;p=strtok(NULL,sep)) {
		if(atoi(p)>0 && atoi(p)<17)
			chns[i]=atoi(p);
		else chns[i] = 1;
		i++;
	}
	

	for(i=0;i<voMutx;i++){
		memset((u_char *)&netCfg,0,sizeof(struct netConfig_st));
		netCfg.magic=PKG_MAGIC;
		netCfg.chn=i;
		netCfg.subwin=voMutx;

		sprintf(netCfg.mediaInfo.camAddress,"%s",env->camConn[chns[i]-1].address);
		if(voMutx==1 || voMutx ==4)
			sprintf(netCfg.mediaInfo.camUrl,"/h264/ch1/main/av_stream");
		else
			sprintf(netCfg.mediaInfo.camUrl,"/h264/ch1/sub/av_stream");
		sprintf(netCfg.mediaInfo.camUser,"admin");
		sprintf(netCfg.mediaInfo.camPasswd,"fhjt12345");
		netCfg.mediaInfo.camPort =554;

		printf("cam ip :%s \r\n",netCfg.mediaInfo.camAddress);
		//memset(netCfg.mediaInfo.camAddress,0,32);
		c->send(rtspConn,(u_char *)&netCfg,sizeof(struct netConfig_st));
	}
	return 0;
}
