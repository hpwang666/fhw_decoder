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
	sprintf(sql,"select * from camera");
	result = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if(result != SQLITE_OK )
	{
		printf( " prepare 错误码:%d，错误原因:%s\r\n", result, errmsg );
	}
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		printf("id:%d address: %s, ip:%s,  type: %d\n",\
				sqlite3_column_int(stmt, 0),\
				sqlite3_column_text(stmt, 1),\
				sqlite3_column_text(stmt, 2),\
				sqlite3_column_int(stmt, 3));

		camConn=&(ev->camConn[sqlite3_column_int(stmt, 0)-1]);
		sprintf(camConn->address,"%s",sqlite3_column_text(stmt, 1));
		if((camConn->ct) != NULL) {httpclientFree(camConn->ct); camConn->ct=NULL;}
		if(strncmp(camConn->address,"0.0.0.0",7)!=0){
			camConn->ct = httpClientCreat(camConn->address,"admin","fhjt12345");
			if(getCamName(camConn))
				debug("channel[%d]:%s,name:%s\r\n",i,camConn->address,camConn->camName);
		}
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


	sqlite3_close( db );
	return 0;
}


int getCamName(camConnection camConn)
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
