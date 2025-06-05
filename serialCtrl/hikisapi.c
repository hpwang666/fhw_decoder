#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "httpclient.h"
#include "hikisapi.h"
#include <sqlite3.h>
#undef  _DEBUG
#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif




int getChnnelInfo(loop_ev ev)
{
	sqlite3 * db = NULL; //声明sqlite关键结构指针
	int result;
	int i=0;
	char * errmsg = NULL;
	sqlite3_stmt *stmt = NULL;
	char sql[128];
	camConnection camConn;
	//需要传入 db 这个指针的指针，因为 sqlite3_open 函数要为这个指针分配内存，还要让db指针指向这个内存区
	result = sqlite3_open( "/mnt/usr/ss.db",&db );

	if( result != SQLITE_OK )
	{
		printf("can not open the database %s\r\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}


	sprintf(sql,"select * from controller where id = 1");
	result = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if(result != SQLITE_OK )
	{
		printf( " prepare 错误码:%d，错误原因:%s\r\n", result, errmsg );
	}
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		printf("id:%d h_type: %d,muxt4:%d passwd:%s\n",\
				sqlite3_column_int(stmt, 0),\
				sqlite3_column_int(stmt, 1),\
				sqlite3_column_int(stmt, 2),\
				sqlite3_column_text(stmt, 4));\

		ev->decType=sqlite3_column_int(stmt, 1);
		ev->muxt4=sqlite3_column_int(stmt, 2);
		sprintf(ev->passwd,"%s",sqlite3_column_text(stmt, 4));
	}
	sqlite3_finalize(stmt);
	

	sprintf(sql,"select * from camera");
	result = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if(result != SQLITE_OK )
	{
		printf( " prepare 错误码:%d，错误原因:%s\r\n", result, errmsg );
	}
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		printf("id:%d address: %s, url:%s,  type: %d\n",\
				sqlite3_column_int(stmt, 0),\
				sqlite3_column_text(stmt, 1),\
				sqlite3_column_text(stmt, 2),\
				sqlite3_column_int(stmt, 3));

		camConn=&(ev->camConn[sqlite3_column_int(stmt, 0)-1]);
		sprintf(camConn->address,"%s",sqlite3_column_text(stmt, 1));
		sprintf(camConn->url,"%s",sqlite3_column_text(stmt, 2));
		if((camConn->ct) != NULL) {httpclientFree(camConn->ct); camConn->ct=NULL;}
		if(strncmp(camConn->address,"0.0.0.0",7)!=0){
			camConn->ct = httpClientCreat(camConn->address,"admin",ev->passwd);
			if(getCamName(ev,camConn))
				debug("channel[%d]:%s,name:%s\r\n",i,camConn->address,camConn->camName);

			//这里获取通道0的初始ptz位置
			if(sqlite3_column_int(stmt, 0)==1){
				
				//getPtzStatus(ev,camConn);
				//直接获取r0 r1 寄存器  手动配置
			}
		}
	}
	sqlite3_finalize(stmt);


	sprintf(sql,"select * from event");
	result = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if(result != SQLITE_OK )
	{
		printf( " prepare 错误码:%d，错误原因:%s\r\n", result, errmsg );
	}
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		printf("id:%d alarm_ip: %s,alarm_port:%d ,event_type:%d \n",\
				sqlite3_column_int(stmt, 0),\
				sqlite3_column_text(stmt, 1),\
				sqlite3_column_int(stmt, 2),\
				sqlite3_column_int(stmt, 3));

		sprintf(ev->alarm_ip,"%s",sqlite3_column_text(stmt, 1));
		ev->alarmPort=sqlite3_column_int(stmt, 2);
		ev->event_type  = sqlite3_column_int(stmt, 3);
		ev->ch0_azimuth= sqlite3_column_int(stmt, 4);//对应P
		ev->ch0_elevation= sqlite3_column_int(stmt, 5);//对应T
	}
	sqlite3_finalize(stmt);


	sprintf(sql,"select * from plc_ctrl");
	result = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if(result != SQLITE_OK )
	{
		printf( " prepare 错误码:%d，错误原因:%s\r\n", result, errmsg );
	}
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		printf("id:%d cmd: %s,vo:%d , cameras:%s  \n",\
				sqlite3_column_int(stmt, 0),\
				sqlite3_column_text(stmt, 1),\
				sqlite3_column_int(stmt, 2),\
				sqlite3_column_text(stmt, 3));\
		sprintf(ev->plcCams[sqlite3_column_int(stmt, 0)-1],"%s",sqlite3_column_text(stmt, 3));
	}
	sqlite3_finalize(stmt);




	sprintf(sql,"select * from plc_info where id = 1");
	result = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if(result != SQLITE_OK )
	{
		printf( " prepare 错误码:%d，错误原因:%s\r\n", result, errmsg );
	}
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		printf("id:%d addr: %s,port:%d ,r0:%d,r1:%d,protocol:%d \n",\
				sqlite3_column_int(stmt, 0),\
				sqlite3_column_text(stmt, 1),\
				sqlite3_column_int(stmt, 2),\
				sqlite3_column_int(stmt, 3),\
				sqlite3_column_int(stmt, 4),\
				sqlite3_column_int(stmt, 5));

		sprintf(ev->plcAddr,"%s",sqlite3_column_text(stmt, 1));
		ev->plcPort=sqlite3_column_int(stmt, 2);
		ev->r0=sqlite3_column_int(stmt, 3);
		ev->r1=sqlite3_column_int(stmt, 4);
		ev->protocol=sqlite3_column_int(stmt, 5);
		
	}

	printf("%s:%d\r\n",ev->plcAddr,ev->plcPort);
	printf("Firmware compile time:%s %s\r\n",__DATE__,__TIME__);
	sprintf(sql,"update controller set version= \"%s %s\" where id = 1",__DATE__,__TIME__);
	result= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
	if(result!= SQLITE_OK )
	{
		printf( "更新失败，错误码:%d，错误原因:%s\r\n", result, errmsg );
	}
	sqlite3_close( db );
	return 0;
}


int getCamName(loop_ev ev,camConnection camConn)
{
	char *head=NULL;
	char *tail=NULL;
	int ret;
	memset(camConn->camName,'\0',32);
	ret=httpClientGet(camConn->ct,"/ISAPI/System/deviceInfo");
	if(ret==-404)
	{
		httpClearConn(camConn->ct);
		ret=httpClientGet(camConn->ct,"/cgi-bin/magicBox.cgi?action=getDeviceType");
		if(ret>0)
		{
			head = strstr(camConn->ct->httpBuf,"type=")+5;
			// if(head==NULL)
			// 	return 0;
			tail = strstr(head,"\r\n");			
			memcpy(camConn->camName,head,(tail - head));
			httpClearConn(camConn->ct);			
			return 1;
		}			
		else
			return 0;
	}else if(ret>0){
		head = strstr(camConn->ct->httpBuf,"model")+6;
		tail = strstr(head,"</model>");
		if((tail - head) < 32)
			memcpy(camConn->camName,head,(tail - head));


		httpClearConn(camConn->ct);
		return 1;
	}else 
		return 0;
}


int getPtzStatus(loop_ev ev,camConnection camConn)
{
	
	char *head=NULL;
	char *tail=NULL;
	char temp[32];
	int ret;

	httpClearConn(camConn->ct);
	ret=httpClientGet(camConn->ct,"/ISAPI/PTZCtrl/channels/1/status");
	printf("ret:%d\r\n",ret);
	if(ret>0){
		head=strstr(camConn->ct->httpBuf,"<elevation>");
		if(head){
			head+=11;
			tail=strstr(head,"</elevation>");
			snprintf(temp,tail-head+1,"%s",head);
			printf("elevation:%s\r\n",temp);
			ev->ch0_elevation=atoi(temp);
		}

		head=strstr(camConn->ct->httpBuf,"<azimuth>");
		if(head){
			head+=9;
			tail=strstr(head,"</azimuth>");
			snprintf(temp,tail-head+1,"%s",head);
			printf("azimuth:%s\r\n",temp);
			ev->ch0_azimuth=atoi(temp);
		}
	}
	httpClearConn(camConn->ct);
	return 1;
}
