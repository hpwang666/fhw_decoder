
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "env.h"

#include "connet.h"
#include "cache.h"
#include "hik_event.h"



static int init_accepted_hik_server_conn(conn_t c,void *arg);
static cache_t hikEventCache ;
static int handWriteBuf2File(readJpeg_t readJpeg);
static int handEventVo(char *eventHost,loop_ev env);
static conn_t alarmConn;

int alarm_read_handle(event_t ev);



int event_read_handle(event_t event)
{
	int r;
	int res;
	int i;
	conn_t c = (conn_t)event->data;
	cacheNode_t node= (cacheNode_t)(c->data);
	readJpeg_t readJpeg = (readJpeg_t)node->data;
	loop_ev env = (loop_ev)readJpeg->env;
	//threadPool_t tp = (threadPool_t)c->data;//lc->ls_arg
	buf_t buf=c->readBuf;
	do{
		buf_extend(buf, 4096);
		r = c->recv(c,buf->tail,4096);
		if(r<=0){
			if(r==0){
				cache_push(hikEventCache, node);
				close_conn(c);
				printf("peer conn closed:%s:%d\n",c->peer_ip,c->peer_port);
			}
			else handle_read_event(event);
		}
		else{
			buf->size += r;
			buf->tail += r;
			if(128*1024>readJpeg->bufIndex){
				if(r<(128*1024-readJpeg->bufIndex))
					memcpy(readJpeg->readBuf+readJpeg->bufIndex,buf->head,r);
			}
			readJpeg->bufIndex+=r;
			//printf("recv>>%d\n",r);
			if(readJpeg->contentLen[0]==0||(readJpeg->bufIndex == readJpeg->headLength+4+readJpeg->contentLen[0])){
				res = handWriteBuf2File(readJpeg);
				if(res == 0){
					if(alarmConn)
						handle_write_event(alarmConn->write);//向报警主机发送指令
					handEventVo(c->peer_ip,env);
					memset(readJpeg->readBuf,0,128*1024);
					readJpeg->bufIndex=0;
					readJpeg->headLength=0;
					for(i=0;i<4;i++){
						readJpeg->contentLen[i]=0;
						readJpeg->content[i]=NULL;
					}
				}
			}
			buf_consume(buf, r);
			if(event->ready){
				//printf("ready\n");
				//handle_read_event(ev);//此句注销后，必须用while循环将缓冲区的数据读完，这样好处就是  减少了event触发次数。
			}
		}
	}while(event->ready);
	
	//handle_read_event(ev);//此句只有在缓存数据没读完才会触发，以上的循环已经把数据读完了
	return 0;
}



int alarm_read_handle(event_t ev)
{
	int r;
	
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
			
			buf_consume(buf, r);
	//		pc->send(pc,(u_char *)"ok ok ok~~",10);
		}
	}while(ev->ready);
	return 0;
}


int alarm_write_handle(event_t ev)
{
	int n;
	conn_t c =(conn_t) ev->data;
	n = c->send(c,(u_char *)"$$060401",8);
	if(!c->write->ready) printf("only send %d bytes\n",n);
	return 0;
}

static int init_accepted_hik_server_conn(conn_t c,void *arg)
{
	int i=0;
	printf("cache size %d\r\n",hikEventCache->cacheSize);
	cacheNode_t node =(cacheNode_t) cache_pull(hikEventCache);
	readJpeg_t readJpeg= (readJpeg_t)(node->data);
	memset(readJpeg->readBuf,0,128*1024);
	readJpeg->bufIndex=0;
	readJpeg->env=arg;//loop_ev
	readJpeg->headLength=0;
	for(i=0;i<4;i++){
		readJpeg->contentLen[i]=0;
		readJpeg->content[i]=NULL;
	}
	c->read->handler = event_read_handle;
	c->data = node;
	printf("Acceped,client=%s:%d peer=%s:%d\n", c->local_ip,c->local_port,c->peer_ip,c->peer_port);
	add_event(c->read,READ_EVENT);
	
	return 0;
}

int create_event_server(loop_ev ev)
{
	conn_t lc;
	
	hikEventCache = cache_new(6,sizeof(struct readJpeg_st));
	
	lc = create_listening(8080);//默认8080端口
	lc->ls_handler = init_accepted_hik_server_conn;
	lc->ls_arg = ev;//这里利用lc将参数最终传递给所有的c->data
	
	return 0;
}


int create_alarm_client(loop_ev env)
{
	if(strncmp(env->alarm_ip,"0.0.0.0",7)==0){
		env->alarmConn = NULL;
		alarmConn=NULL;
		return 0;
	}
	connect_peer_udp(env->alarm_ip,env->alarmPort,&alarmConn);
	alarmConn->read->handler =  alarm_read_handle;
	alarmConn->write->handler = alarm_write_handle;
	alarmConn->data = NULL;
	env->alarmConn = alarmConn;
	return 0;
}

int free_event_server()
{
	cache_free(hikEventCache);
	return 0;
}


static int handWriteBuf2File(readJpeg_t readJpeg)
{
	int i;
	//int res =1;
	u_char *head,*tail;
	char boundary[256];
	char lenChar[32];
	//time_t tm_now;
	//struct tm *p_local_tm;

	if(readJpeg->bufIndex<1000)
		return 1;
	head = str_nstr(readJpeg->readBuf,"boundary=",readJpeg->bufIndex);
	if(head){
		tail=str_nstr(head,"\r\n",64);
		if(tail){
			snprintf(boundary,tail-head-9+1,"%s",head+9);
			printf("boundary=%s\r\n",boundary);
			for(i=0;i<2;i++){//只要了前两个 content-length
				head = str_nstr(tail,"Content-Length: ",readJpeg->bufIndex-(tail-readJpeg->readBuf));
				if(head){
					tail = str_nstr(head,"\r\n",64);
					if(tail){
						snprintf(lenChar,tail-head-16+1,"%s",head+16);
						readJpeg->contentLen[i]= atoi(lenChar);
						printf("contentLen: %d \r\n",readJpeg->contentLen[i]);
						head = str_nstr(tail,"\r\n\r\n",readJpeg->bufIndex-(tail-readJpeg->readBuf));
						if(head){
							readJpeg->content[i]=head+4;					
							if(i==0){
								readJpeg->headLength=head-readJpeg->readBuf;					
							}
						}
					}
				}
				if(readJpeg->content[i]){
					//1 数组里面是目标描述信息
					//2 数组里面是目标图片
					if(i>0)
						tail=readJpeg->content[i]+readJpeg->contentLen[i]+4;
					else
						tail=readJpeg->content[i];
				}
				else break;
				if(tail==NULL) break;
			}
		}
	}
	if(readJpeg->bufIndex == readJpeg->headLength+4+readJpeg->contentLen[0])//数据接收完成
	{
		printf("ok done\r\n");
		return 0;
	}
	else{
		return 1;
	}
#if 0
	readJpeg->fileLength = readJpeg->contentLen[2];
	time(&tm_now);

	p_local_tm = localtime(&tm_now);
	//printf("time %ld %d \r\n",tm_now,p_local_tm->tm_mday);
	sprintf(readJpeg->fileName,"%d%d-%d%d%d.jpg",p_local_tm->tm_mon+1,p_local_tm->tm_mday,p_local_tm->tm_hour,p_local_tm->tm_min,p_local_tm->tm_sec); 
	readJpeg->fe=fopen(readJpeg->fileName,"wb+");
	if(readJpeg->fe > 0){
		fwrite(readJpeg->content[2],readJpeg->fileLength,1,readJpeg->fe);
		fclose(readJpeg->fe);
		readJpeg->bufIndex=0;
		readJpeg->headLength=0;
		for(i=0;i<4;i++){
			readJpeg->contentLen[i]=0;
			readJpeg->content[i]=NULL;
		}
		memset(readJpeg->readBuf,0,1024*1024);
	}


	return res;
#endif
}



static int handEventVo(char *eventHost,loop_ev env)
{
	int i;
	struct custom_st custom;
	camConnection camConn;
	camConn = env->camConn;
	for(i=0;i<32;i++){
		if(strncmp(eventHost,camConn[i].address,strlen(eventHost))==0) break;
	}
	if(i==32) return 0;

	printf("event address %s\r\n",camConn[i].address);
	custom.ch =i; 
	custom.cmd = 0x1a;
	custom.stop =0;//这里不需要 
	queue_push(env->voQueue,1,sizeof(struct custom_st),&custom);
	return 0;
}
