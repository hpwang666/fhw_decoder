#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "env.h"

#include "connet.h"
#include "plcBus.h"


#undef MODBUS_PRINT
#define MODBUS_PRINT


//读取长度  单位为字
#define MEMOBUS_READ_LEN (1)

//从站通讯地址
#define MEMOBUS_SLAVE_ADDR (7)

//是否打开高度变焦控制
//#define MEMOBUS_HEIGHT


static int plc_connect_handler(event_t ev);
static int plc_reconnect_peer(event_t ev);
uint16_t check_crc(uint8_t* pbuffer, int length);
static int modbus_read_handle(event_t ev);
static int memobus_read_handle(event_t ev);
int handle_memobus(conn_t c,int len);
int handle_modbus(modbusSession_t  modbusSession);
int parseModbus(conn_t c,int len);
int responseModbus( modbusSession_t  modbusSession);

static int init_accepted_modbus_server_conn(conn_t c,void *arg);


extern loop_ev	env;
int memobus_read_handle(event_t ev)
{
	int r;
	//int i=0;
	conn_t c = (conn_t)ev->data;
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
			
			// 00 03 06 00 00 00 00 01 4d 22 33
			//   功能码03 字符长度06 也就是3个字
			handle_memobus(c,r);
#if 0
			for(i=0;i<r;i++)
				printf("%02x ",buf->head[i]);
			printf("\r\n");
#endif
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


static int modbus_read_handle(event_t ev)
{
	int r;
	int ret=0;
	//int i;
	conn_t c = (conn_t)ev->data;
	if (ev->timedout) {//数据读取超时
		close_conn(c);//
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
				close_conn(c);//
				printf("peer conn closed:%s:%d\n",c->peer_ip,c->peer_port);
				return 0;
			}
			else handle_read_event(ev);
		}
		else{
			buf->size += r;
			buf->tail += r;
			
			do{
				ret = parseModbus(c,buf->size);
#if 0
				for(i=0;i<r;i++)
					printf("%02x ",buf->head[i]);
				printf("\r\n");
#endif
				if(ret>3)
					buf_consume(buf, ret);
				else
					buf_init(buf);
			}while(ret>3);
			if(ev->ready){
				printf("ready\n");
			}
		}
	}while(ev->ready);
	//add_timer(c->read,6000);

	handle_read_event(ev);//此句只有在缓存数据没读完才会触发，以上的循环已经把数据读完了
	return 0;
}

int modbus_write_handle(event_t ev)
{
	int n=-1;
	conn_t c = ev->data;
	modbusSession_t modbusSession = (modbusSession_t )c->data;


    if (ev->timedout) {
		ev->timedout = 0;
		add_event(c->write,WRITE_EVENT);
		//通过add_event 来判断是否可写,对于网络拥塞交给TCP自行处理
		//write_handle  会在调用process_event后被清除，所以用timer转一圈
    }
	while(c->write->ready && modbusSession->sendBuf->size){
		n=-1;
		if(modbusSession->sendBuf->size>=4096)	
			n = c->send(c,(u_char *)modbusSession->sendBuf->head,4096);
		else {
			n = c->send(c,(u_char *)modbusSession->sendBuf->head,modbusSession->sendBuf->size);
		}
		if(n>0){
			buf_consume(modbusSession->sendBuf,n);
		}
	}
#if 0
	if(!c->write->ready){
		printf("#");
		fflush(stdout);
	} 
#endif
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

int handle_memobus(conn_t c,int len)
{
	int cmd;	
	struct custom_st custom;
#ifdef MEMOBUS_HEIGHT
	static int zoomCmd=-1;
	int tempHeight;
	uint16_t height_H;
#endif
	static int voCmd=-1;
	buf_t buf=c->readBuf;
	if(len==(3+2+2*MEMOBUS_READ_LEN )){
		if( (buf->head[4]&0x03)^voCmd){
			voCmd=buf->head[4]&0x03;
			switch (voCmd){
				case 0x00:cmd=3;break;
				case 0x01:cmd=1;break;
				case 0x02:cmd=2;break;
				default:cmd=3;break;
			}
			custom.ch =0; 
			custom.cmd = 0xaa;
			custom.stop =cmd;//0x01--LEFT 0X02--RIGHT 0X03--STOP 
			queue_push(env->voQueue,1,sizeof(struct custom_st),&custom);
		}
#ifdef  MEMOBUS_HEIGHT
		if(buf->head[5]&0x80){//负数，在地平面之下  3300是地上的高度
			height_H=(buf->head[5]<<8)&0xff00;
			tempHeight = 5200+(0xffff-height_H-buf->head[6]);
		}
		else{
			height_H=(buf->head[5]<<8)&0xff00;
			tempHeight = 5200-(height_H+buf->head[6]);
		}
		printf("height0:%d\r\n",tempHeight);
		tempHeight=(tempHeight/32)&0xff;
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
#endif
	}
	return 0;
}
int memobus_write_handle(event_t ev)
{
	int n;
	conn_t c =(conn_t) ev->data;
	u_char plcBus[32];
	plcBus[0]=MEMOBUS_SLAVE_ADDR ;
	plcBus[1]=0x03;
	plcBus[2]=(env->r0>>8)&0xff;
	plcBus[3]=(env->r0)&0xff;
	plcBus[4]=0x00;
	plcBus[5]=MEMOBUS_READ_LEN ;//读取多少个字
	*(uint16_t*)(plcBus+ 6) = check_crc(plcBus, 6);
	//for(n=0;n<8;n++){
	//	printf("%02x ",plcBus[n]);
	//}
	//printf("I have sent 8 bytes\r\n");
	n = c->send(c,plcBus,8);
	if(!c->write->ready) printf("only send %d bytes\n",n);
	add_timer(c->read,10000);
	return 0;
}


int parseModbus(conn_t c,int len)
{
	int sendLen =0;
	int pkgLen=0;
	u_char *in = c->readBuf->head;
	modbusSession_t modbusSession = (modbusSession_t )c->data;
	modbusHeader_t modbusHeader = modbusSession->modbusHeader;
	int writeBytes=0;
	if(len<12){
		//printf("modbus pkg < 12 \r\n");
		return 1;
	}
	memcpy(modbusHeader->context,in,2);
	memcpy(modbusHeader->protocol,in+2,2);
	modbusHeader->length = ((0xff00&(*(in+4)))<<8)+*(in+5);
	if(modbusHeader->length>len-6){
		printf("pkg len %d error\r\n",len);
		return 2;
	}
	pkgLen=modbusHeader->length+6;
	modbusHeader->flag=*(in+6);
	modbusHeader->opt = *(in+7);
	if(modbusHeader->opt==0x03){//读寄存器
		modbusHeader->index =((0xff00&(*(in+8)))<<8)+*(in+9);
		modbusHeader->optLen=((0xff00&(*(in+10)))<<8)+*(in+11);
	}
	if(modbusHeader->opt==0x06){//写一个寄存器
		modbusHeader->index =((0xff00&(*(in+8)))<<8)+*(in+9);
		memcpy(modbusSession->holdReg+(modbusHeader->index*2),in+10, 2);
	}
	if(modbusHeader->opt==0x10){//写多个寄存器
		modbusHeader->index =((0xff00&(*(in+8)))<<8)+*(in+9);
		modbusHeader->optLen=((0xff00&(*(in+10)))<<8)+*(in+11);
		writeBytes=*(in+12);
		if( writeBytes>len-13){
			printf("write bytes err %d\r\n", writeBytes);
			return 3;
		}
		memcpy(modbusSession->holdReg+(modbusHeader->index*2),in+13, writeBytes);
	}
	//printf("modbus header len:%d data len %d\r\n",modbusHeader->length,modbusHeader->optLen);
	sendLen = responseModbus(modbusSession);
	if(sendLen){
		add_event(c->write, WRITE_EVENT);
	}
#ifdef MODBUS_PRINT 
	int i;
	printf("hold reg: ");
	for(i=0;i<10;i++)
		printf("%02x ",*(modbusSession->holdReg+i));
	printf("\r\n");
#endif
	if(modbusHeader->opt==0x06 ||modbusHeader->opt==0x10)//写寄存器之后才会进行vo ptz操作
		handle_modbus(modbusSession);

	return pkgLen;
}
int responseModbus( modbusSession_t  modbusSession)
{
	int sendLen =0;
	u_char sendData[1024];
	modbusHeader_t modbusHeader = modbusSession->modbusHeader;
	if(modbusHeader->opt==0x03){
		memcpy(sendData+sendLen,modbusHeader->context,2);
		sendLen+=2;
		memcpy(sendData+sendLen,modbusHeader->protocol,2);
		sendLen+=2;
		*(sendData+sendLen)=((modbusHeader->optLen*2+3)>>8)&0x00ff;
		sendLen+=1;
		*(sendData+sendLen)=(modbusHeader->optLen*2+3)&0x00ff;
		sendLen+=1;
		*(sendData+sendLen)=modbusHeader->flag;
		sendLen+=1;
		*(sendData+sendLen)=modbusHeader->opt;
		sendLen+=1;
		*(sendData+sendLen)=modbusHeader->optLen*2;
		sendLen+=1;
		memcpy(sendData+sendLen,modbusSession->holdReg+(modbusHeader->index*2),modbusHeader->optLen*2);
		sendLen+=modbusHeader->optLen*2;
	}
	if(modbusHeader->opt==0x10){
		memcpy(sendData+sendLen,modbusHeader->context,2);
		sendLen+=2;
		memcpy(sendData+sendLen,modbusHeader->protocol,2);
		sendLen+=2;
		*(sendData+sendLen)=((6)>>8)&0x00ff;
		sendLen+=1;
		*(sendData+sendLen)=(6)&0x00ff;
		sendLen+=1;
		*(sendData+sendLen)=modbusHeader->flag;
		sendLen+=1;
		*(sendData+sendLen)=modbusHeader->opt;
		sendLen+=1;
		*(sendData+sendLen)=((modbusHeader->index)>>8)&0x00ff;
		sendLen+=1;
		*(sendData+sendLen)=(modbusHeader->index)&0x00ff;
		sendLen+=1;
		*(sendData+sendLen)=((modbusHeader->optLen)>>8)&0x00ff;
		sendLen+=1;
		*(sendData+sendLen)=(modbusHeader->optLen)&0x00ff;
		sendLen+=1;
	}
	if(modbusHeader->opt==0x06){
		memcpy(sendData+sendLen,modbusHeader->context,2);
		sendLen+=2;
		memcpy(sendData+sendLen,modbusHeader->protocol,2);
		sendLen+=2;
		*(sendData+sendLen)=((6)>>8)&0x00ff;
		sendLen+=1;
		*(sendData+sendLen)=(6)&0x00ff;
		sendLen+=1;
		*(sendData+sendLen)=modbusHeader->flag;
		sendLen+=1;
		*(sendData+sendLen)=modbusHeader->opt;
		sendLen+=1;
		*(sendData+sendLen)=((modbusHeader->index)>>8)&0x00ff;
		sendLen+=1;
		*(sendData+sendLen)=(modbusHeader->index)&0x00ff;
		sendLen+=1;
		memcpy(sendData+sendLen,modbusSession->holdReg+(modbusHeader->index*2),2);
		sendLen+=2;
	}
	if(sendLen){
		buf_extend(modbusSession->sendBuf,sendLen);
		memcpy(modbusSession->sendBuf->tail,sendData,sendLen);
		modbusSession->sendBuf->size+=sendLen;
		modbusSession->sendBuf->tail+=sendLen;
	}
	return sendLen;	
}
int handle_modbus(modbusSession_t modbusSession)
{
	int cmd;	
	struct custom_st custom;
#if 0
	static int zoomCmd=-1;
	int tempHeight;
	uint16_t height_H;
#endif
	static int voCmd=-1;
	if(modbusSession->holdReg[1]==0x01) cmd=0x01;
	else if(modbusSession->holdReg[1]==0x02) cmd=0x02;
	else if(modbusSession->holdReg[1]==0x04) cmd=0x03;
	else if(modbusSession->holdReg[1]==0x00) cmd=voCmd;
	else cmd=0x03;

	if(cmd^voCmd){
		voCmd=cmd;
		custom.ch =0; 
		custom.cmd = 0xaa;
		custom.stop =cmd;//0x01--LEFT 0X02--RIGHT 0X03--STOP 
		queue_push(env->voQueue,1,sizeof(struct custom_st),&custom);
	}
#if 0
	if(modbusSession->holdReg[2]&0x80){//负数，在地平面之下  2000是地上的高度
		height_H=(modbusSession->holdReg[2]<<8)&0xff00;
		tempHeight =3200+(0xffff-height_H-modbusSession->holdReg[3]);
	}
	else{
		height_H=(modbusSession->holdReg[2]<<8)&0xff00;
		tempHeight =3200-(height_H+modbusSession->holdReg[3]);
	}
	//printf("height0:%d\r\n",tempHeight);
	tempHeight=(tempHeight/64)&0xff;
	//printf("height1:%d\r\n",tempHeight);
	if( tempHeight^zoomCmd){
		zoomCmd=tempHeight;
		cmd = tempHeight;
		custom.ch = 0;	
		custom.cmd = 0xaa;
		custom.stop =cmd; //针对2507 这里从0--250 代表充1-25倍变焦
		queue_push(env->ptzQueue,1,sizeof(struct custom_st),&custom);

		//把画面切换到1通道
		custom.ch =0; 
		custom.cmd = 0x1a;
		custom.stop =0;//这里不需要 
		queue_push(env->voQueue,1,sizeof(struct custom_st),&custom);
	}
#endif
	return 0;
}

static int readCamEvents(event_t ev)
{
	queueNode_t tmp;
	modbusSession_t modbusSession =(modbusSession_t )ev->data;

	if (ev->timedout) {//
        //printf("HTTP WRITE:read timeout\r\n");
		//close_conn(c);//此处不能close，会释放conn，引起错误指针
		ev->timedout = 0;
		
		//return AIO_ERR;
    }
	
	tmp = queue_get(env->eventQueue);//接受相机推送过来的事件
	if(tmp){
		event2plc_t event2plc= (event2plc_t)(tmp->data);
		printf("event ch %d\r\n",event2plc->ch);
		switch( event2plc->ch){
			case 10:event2plc->event==0x01?(modbusSession->holdReg[0]|=0x01):(modbusSession->holdReg[0]&=~0x01);break;//左行海侧报警
			case 5:event2plc->event==0x01?(modbusSession->holdReg[0]|=0x02):(modbusSession->holdReg[0]&=~0x02);break;//左行陆侧报警
			case 11:event2plc->event==0x01?(modbusSession->holdReg[0]|=0x04):(modbusSession->holdReg[0]&=~0x04);break;//右行海侧报警
			case 6:event2plc->event==0x01?(modbusSession->holdReg[0]|=0x08):(modbusSession->holdReg[0]&=~0x08);break;//右行陆侧报警
		}
		queue_cache(env->eventQueue,tmp);
	} 
modbusSession->holdReg[0]|=0x08;
	add_timer(modbusSession->modbusEv,80);

	return 0;
}



int initPlcBus()
{
	conn_t  pc,lc;
	int ret;


	if(env->protocol==2){
		printf("create memobus client\r\n");
		ret = connect_peer(env->plcAddr,env->plcPort,&pc);
		pc->data = env;//传入参数
		if(ret == AIO_AGAIN){
			pc->read->handler = plc_connect_handler;
			pc->write->handler = plc_connect_handler;//write超时函数
			add_timer(pc->write, 1000);//加入定时器去判断
		}
		if(ret == AIO_OK);
	}
	if(env->protocol==1){
		printf("create modbus server port:%d\r\n",env->plcPort);
		lc = create_listening(env->plcPort);//
		env->modbusListenConn=lc;
		lc->ls_handler = init_accepted_modbus_server_conn;
		modbusSession_t modbusSession =(modbusSession_t ) calloc(1,sizeof(struct modbusSession_st));
		modbusSession->modbusHeader =(modbusHeader_t) calloc(1,sizeof(struct modbusHeader_st));
		modbusSession->sendBuf= buf_new(2048);
		modbusSession->modbusEv=(event_t)calloc(1,sizeof(struct event_st));
		modbusSession->modbusEv->handler=readCamEvents;
		modbusSession->modbusEv->data=modbusSession;
		add_timer(modbusSession->modbusEv,100);

		lc->ls_arg = modbusSession;//这里利用lc将参数最终传递给所有的c->data
	}

	return 0;
}
int releasePlcBus()
{
	if(env->protocol==1){
		conn_t c = env->modbusListenConn;
		modbusSession_t modbusSession = (modbusSession_t )c->ls_arg;
		modbusHeader_t modbusHeader = modbusSession->modbusHeader;
		if(modbusSession->modbusEv->timer_set)
			del_timer(modbusSession->modbusEv);
		free(modbusSession->modbusEv);		
		buf_free(modbusSession->sendBuf);
		free(modbusHeader);
		free(modbusSession);
		close_conn(c);
	}
	return 0;
}

static int init_accepted_modbus_server_conn(conn_t c,void *arg)
{
	c->read->handler = modbus_read_handle;
	c->write->handler = modbus_write_handle;
	c->data = (modbusSession_t)arg;//env
	printf("PLC client=%s:%d peer=%s:%d\n", c->local_ip,c->local_port,c->peer_ip,c->peer_port);
	add_event(c->read,READ_EVENT);
	handle_write_event(c->write);
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
	c->read->handler =memobus_read_handle;
	c->write->handler = memobus_write_handle;
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
