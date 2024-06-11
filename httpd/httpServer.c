#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include "httpConfig.h" 
#include "httpServer.h"

#include <sqlite3.h>
#include "connet.h"


#undef  _DEBUG
#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif


#define tPrint(x) \
	snprintf(testPrint,1023,"%s",x);\
	printf(">>>%s\r\n",testPrint);


static http_trans_control trans_c;

char send_data[2048]={0};/*上传的数值*/
uint8_t pub_buf[512];

#define INDEX_HTML "htdocs/index.html"
#define SCENE_HTML "htdocs/scene.html"
JSONPOST_CFG json_config;//解码器的配置信息

uint8_t login_flag = 0;

/**
*@brief		将基本的配置信息设置到json_callback
*@param		无
*@return	无
*/
void make_basic_config_setting_json_callback(char* buf, char* config_msg)
{
#if 0
  sprintf(buf,"settingsCallback({\"ver\":\"%d.%d\",\
                \"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\
                \"ip\":\"%d.%d.%d.%d\",\
                \"gw\":\"%d.%d.%d.%d\",\
                \"sub\":\"%d.%d.%d.%d\",\
                });",config_msg.sw_ver[0],config_msg.sw_ver[1],
                config_msg.mac[0],config_msg.mac[1],config_msg.mac[2],config_msg.mac[3],config_msg.mac[4],config_msg.mac[5],
                config_msg.lip[0],config_msg.lip[1],config_msg.lip[2],config_msg.lip[3],
                config_msg.gw[0],config_msg.gw[1],config_msg.gw[2],config_msg.gw[3],
                config_msg.sub[0],config_msg.sub[1],config_msg.sub[2],config_msg.sub[3]
         );
#endif
}
void make_post_config_setting_json_callback(char* buf, JSONPOST_CFG config_msg)
{
	char address[32][32];
	char plcCtrl[3][64];
	char version[128];
	u_char muxt4=0;
    u_char dec=0; 	
	char passwd[32];
	char alarm_ip[32];
	int alarmPort=0;
	int eventType=0;
	int hik_p=0;
	int hik_t=0;

	char plcAddr[32];
	int plcPort=0;
	int r0=0;
	int r1=0;
	int protocol=0;
	sqlite3 * db = NULL; //声明sqlite关键结构指针
	int result;
	int i=0;
	char * errmsg = NULL;
	sqlite3_stmt *stmt = NULL;
	char sql[128];
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
		sprintf(address[sqlite3_column_int(stmt, 0)-1],"%s",sqlite3_column_text(stmt, 1));
	}
	sqlite3_finalize(stmt);

	sprintf(sql,"select * from plc_ctrl where id <4");
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
		sprintf(plcCtrl[sqlite3_column_int(stmt, 0)-1],"%s",sqlite3_column_text(stmt, 3));
	}
	sqlite3_finalize(stmt);


	sprintf(sql,"select * from controller where id = 1");
	result = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if(result != SQLITE_OK )
	{
		printf( " prepare 错误码:%d，错误原因:%s\r\n", result, errmsg );
	}
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		printf("id:%d h_type: %d,muxt4:%d ,version:%s\n",\
				sqlite3_column_int(stmt, 0),\
				sqlite3_column_int(stmt, 1),\
				sqlite3_column_int(stmt, 2),\
				sqlite3_column_text(stmt, 3));\

			muxt4=sqlite3_column_int(stmt, 2);
		dec=sqlite3_column_int(stmt, 1);
		sprintf(version,"%s",sqlite3_column_text(stmt, 3));
		sprintf(passwd,"%s",sqlite3_column_text(stmt, 4));
	}
	sqlite3_finalize(stmt);


	sprintf(sql,"select * from event where id = 1");
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

		sprintf(alarm_ip,"%s",sqlite3_column_text(stmt, 1));
		alarmPort=sqlite3_column_int(stmt, 2);
		eventType=sqlite3_column_int(stmt, 3);
		hik_p=sqlite3_column_int(stmt, 4);
		hik_t=sqlite3_column_int(stmt, 5);
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

		sprintf(plcAddr,"%s",sqlite3_column_text(stmt, 1));
		plcPort=sqlite3_column_int(stmt, 2);
		r0=sqlite3_column_int(stmt, 3);
		r1=sqlite3_column_int(stmt, 4);
		protocol=sqlite3_column_int(stmt, 5);
	}
	sqlite3_finalize(stmt);
	sqlite3_close( db );
  sprintf(buf,"postConfigCallback({\
				\"stream1\":\"%s\",\
				\"stream2\":\"%s\",\
				\"stream3\":\"%s\",\
				\"stream4\":\"%s\",\
				\"stream5\":\"%s\",\
				\"stream6\":\"%s\",\
				\"stream7\":\"%s\",\
				\"stream8\":\"%s\",\
				\"stream9\":\"%s\",\
				\"stream10\":\"%s\",\
				\"stream11\":\"%s\",\
				\"stream12\":\"%s\",\
				\"stream13\":\"%s\",\
				\"stream14\":\"%s\",\
				\"stream15\":\"%s\",\
				\"stream16\":\"%s\",\
				\"stream17\":\"%s\",\
				\"stream18\":\"%s\",\
				\"stream19\":\"%s\",\
				\"stream20\":\"%s\",\
				\"stream21\":\"%s\",\
				\"stream22\":\"%s\",\
				\"stream23\":\"%s\",\
				\"stream24\":\"%s\",\
				\"stream25\":\"%s\",\
				\"stream26\":\"%s\",\
				\"stream27\":\"%s\",\
				\"stream28\":\"%s\",\
				\"stream29\":\"%s\",\
				\"stream30\":\"%s\",\
				\"stream31\":\"%s\",\
				\"stream32\":\"%s\",\
				\"left\":\"%s\",\
				\"right\":\"%s\",\
				\"stop\":\"%s\",\
				\"muxt4\":\"%d\",\
				\"dec\":\"%d\",\
				\"cam_passwd\":\"%s\",\
				\"alarm_ip\":\"%s\",\
				\"alarm_port\":\"%d\",\
				\"event_type\":\"%d\",\
				\"hik_p\":\"%d\",\
				\"hik_t\":\"%d\",\
				\"version\":\"%s\",\
				\"plc_addr\":\"%s\",\
				\"plc_port\":\"%d\",\
				\"r0\":\"%d\",\
				\"r1\":\"%d\",\
				\"protocol\":\"%d\",\
                });",\
				address[0],address[1],address[2],address[3],\
				address[4],address[5],address[6],address[7],\
				address[8],address[9],address[10],address[11],\
				address[12],address[13],address[14],address[15],\
				address[16],address[17],address[18],address[19],\
				address[20],address[21],address[22],address[23],\
				address[24],address[25],address[26],address[27],\
				address[28],address[29],address[30],address[31],\
				plcCtrl[0],plcCtrl[1],plcCtrl[2],muxt4,dec,passwd,alarm_ip,alarmPort,eventType,\
				hik_p,hik_t,version,\
  				plcAddr,plcPort,r0,r1,protocol
         );
		
}


void make_get_scene_json_callback(char* buf, JSONPOST_CFG config_msg)
{
	char cams[8][64];
	char name[8][64];
	sqlite3 * db = NULL; //声明sqlite关键结构指针
	int result;
	int i=0;
	char * errmsg = NULL;
	sqlite3_stmt *stmt = NULL;
	char sql[128];
	//需要传入 db 这个指针的指针，因为 sqlite3_open 函数要为这个指针分配内存，还要让db指针指向这个内存区
	result = sqlite3_open( "/mnt/usr/ss.db",&db );

	if( result != SQLITE_OK )
	{
		printf("can not open the database %s\r\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}
	sprintf(sql,"select * from plc_ctrl where id >3");
	result = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if(result != SQLITE_OK )
	{
		printf( " prepare 错误码:%d，错误原因:%s\r\n", result, errmsg );
	}
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		printf("id:%d cmd:%s,vo:%d , cameras:%s  \n",\
				sqlite3_column_int(stmt, 0),\
				sqlite3_column_text(stmt, 1),\
				sqlite3_column_int(stmt, 2),\
				sqlite3_column_text(stmt, 3));\
		sprintf(cams[sqlite3_column_int(stmt, 0)-4],"%s",sqlite3_column_text(stmt, 3));
		sprintf(name[sqlite3_column_int(stmt, 0)-4],"%s",sqlite3_column_text(stmt, 1));
	}
	sqlite3_finalize(stmt);


	sqlite3_close( db );
  sprintf(buf,"getSceneCallback({\
				\"name4\":\"%s\",\
				\"name5\":\"%s\",\
				\"name6\":\"%s\",\
				\"name7\":\"%s\",\
				\"name8\":\"%s\",\
				\"name9\":\"%s\",\
				\"name10\":\"%s\",\
				\"name11\":\"%s\",\
				\"cams4\":\"%s\",\
				\"cams5\":\"%s\",\
				\"cams6\":\"%s\",\
				\"cams7\":\"%s\",\
				\"cams8\":\"%s\",\
				\"cams9\":\"%s\",\
				\"cams10\":\"%s\",\
				\"cams11\":\"%s\",\
                });",\
				name[0],name[1],name[2],name[3],name[4],name[5],name[6],name[7],\
				cams[0],cams[1],cams[2],cams[3],cams[4],cams[5],cams[6],cams[7]\
         );
		
}

uint8_t set_scene_process(http_request_t http_request)
{
	
	uint8_t ret=0;
	uint8_t * param;
	uint8_t i=0;	
	char name[16];
	sqlite3 * db = NULL; //声明sqlite关键结构指针
	char * errmsg = NULL;
	ret= sqlite3_open( "/mnt/usr/ss.db",&db );
	char sql[256];

	char cams[64];

	if( ret!= SQLITE_OK )
	{
		printf("can not open the database %s\r\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	for(i=4;i<12;i++){
		printf("%d\r\n",i);
		sprintf(name,"name%d",i);
		param = get_http_param_value(http_request->URI,name);		
		if(!strlen(param)) {
			printf("param %s not found\r\n",name);
			continue;
		}
		sprintf(sql,"update plc_ctrl set cmd= \"%s\" where id = %d",param,i);
		printf("  修改后的 cmd %d sql: %s \r\n",i,param);
		ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
		if(ret!= SQLITE_OK )
		{
			printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}


		sprintf(cams,"cams%d",i);
		param = get_http_param_value(http_request->URI,cams);		/*获取修改后的IP地址*/
		if(!strlen(param)) {
			printf("param %s not found\r\n",cams);
			continue;
		}
		if(verify_plc_ctrl(param)){
			sprintf(sql,"update plc_ctrl set cameras= \"%s\" where id = %d",param,i);
			printf("  修改后的 cams %d sql: %s \r\n",i,param);
			ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
			if(ret!= SQLITE_OK )
			{
				printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
			}
		}
		else{
			printf("error: %s is invaliade \r\n",param);
		}	
	}



	sqlite3_close( db );
	return 1;
}
/**
*@brief		将配置信息写进单片机eeprom
*@param		http_request：定义一个http请求的结构体指针
*@return	无
*/
void cgi_ipconfig(http_request_t http_request)
{ 
	//	write_config_to_eeprom();																/*将获取的网络参数写进eeprom*/
}
void cgi_postset(http_request_t http_request)
{
	uint8_t i;
	uint8_t temp_str[10]="stream";
	uint8_t * param;
	JSONEEPROM_CFG eeprom_cfg;
	

}
char cgi_test_process(http_request_t http_request)
{ 
	char * param;
	uint16_t content_len=0;
	char tmp_buf[10]={0x00,};
	param = http_request->URI;


	if(!param ) return 0;
	/***************/
	mid(param,"Content-Length: ","\r\n",tmp_buf);
	content_len=atoi16(tmp_buf,10);
	param = (char*)strstr(param,"\r\n\r\n");
	param+=4;
//	printf("act=%s\r\n",param+content_len-1);//
	return *(param+content_len-1);
}
/**
*@brief		执行http响应
*@param		无  
*@return	无
*/
void make_cgi_response(uint16_t delay, char* url,char* cgi_response_buf)
{
  sprintf(cgi_response_buf,"<html><head><title>Configuration of embedded message server</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j + ' ';j--;setTimeout('func()',1000);if(j==0)location.href='http://%d.%d.%d.%d/';}</script></head><body onload='func()'>please wait for a while, the module will reboot in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay,1,1,1,1);
  return;
}

uint8_t cgi_geshihua_process(http_request_t http_request)
{
	uint8_t ret=0;
	uint8_t * param;
	uint8_t i=0;	
	char stream[16];
	sqlite3 * db = NULL; //声明sqlite关键结构指针
	char * errmsg = NULL;
	ret= sqlite3_open( "/mnt/usr/ss.db",&db );
	char sql[256];


	char alarm_ip[32];
	int alarmPort=0;
	int eventType=0;

	if( ret!= SQLITE_OK )
	{
		printf("can not open the database %s\r\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	for(i=0;i<32;i++){
		sprintf(stream,"stream%d",i+1);
		param = get_http_param_value(http_request->URI,stream);		/*获取修改后的IP地址*/
		if(!param) {
			printf("param %s not found\r\n",stream);
			return -1;
		}
		if(verify_ip_address(param)){
			sprintf(sql,"update camera set address = \"%s\" where id = %d",param,i+1);
			printf("  修改后的 ch %d sql: %s \r\n",i+1,sql);
			ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
			if(ret!= SQLITE_OK )
			{
				printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
			}
		}
		
	}


	sprintf(stream,"left");
	param = get_http_param_value(http_request->URI,stream);		/*获取修改后的IP地址*/
	if(verify_plc_ctrl(param)){
		sprintf(sql,"update plc_ctrl set cameras= \"%s\" where id = 1",param);
		ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
		if(ret!= SQLITE_OK )
		{
			printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}
	}
	else printf("left formate error \r\n");
	sprintf(stream,"right");
	param = get_http_param_value(http_request->URI,stream);		/*获取修改后的IP地址*/
	if(verify_plc_ctrl(param)){
		sprintf(sql,"update plc_ctrl set cameras= \"%s\" where id = 2",param);
		ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
		if(ret!= SQLITE_OK )
		{
			printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}
	}
	else printf("right formate error \r\n");


	sprintf(stream,"stop");
	param = get_http_param_value(http_request->URI,stream);		/*获取修改后的IP地址*/
	if(verify_plc_ctrl(param)){
		sprintf(sql,"update plc_ctrl set cameras= \"%s\" where id = 3",param);
		ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
		if(ret!= SQLITE_OK )
		{
			printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}
	}
	else printf("stop formate error \r\n");


	sprintf(stream,"muxt4");
	param = get_http_param_value(http_request->URI,stream);		/*获取修改后的IP地址*/
	sprintf(sql,"update controller set muxt4_type= \"%s\" where id = 1",param);
	ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
	if(ret!= SQLITE_OK )
	{
		printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
	}


	sprintf(stream,"dec");
	param = get_http_param_value(http_request->URI,stream);		/*获取修改后的IP地址*/
	sprintf(sql,"update controller set h_type= \"%s\" where id = 1",param);
	ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
	if(ret!= SQLITE_OK )
	{
		printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}


	sprintf(stream,"cam_passwd");
	param = get_http_param_value(http_request->URI,stream);		/*获取修改后的IP地址*/
	sprintf(sql,"update controller set passwd= \"%s\" where id = 1",param);
	ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
	if(ret!= SQLITE_OK )
	{
		printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}


	sprintf(stream,"alarm_ip");
	param = get_http_param_value(http_request->URI,stream);		
	if(verify_ip_address(param)){
		sprintf(sql,"update event set alarm_ip= \"%s\" where id = 1",param);
		ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
		if(ret!= SQLITE_OK )
		{
			printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}
	}


	sprintf(stream,"alarm_port");
	param = get_http_param_value(http_request->URI,stream);		
	if(verify_num(param)&&(atoi(param)<20000)&&(atoi(param)>100)){
		sprintf(sql,"update event set alarm_port= \"%s\" where id = 1",param);
		ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
		if(ret!= SQLITE_OK )
		{
			printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}
	}

	sprintf(stream,"event_type");
	param = get_http_param_value(http_request->URI,stream);		
	sprintf(sql,"update event set event_type= \"%s\" where id = 1",param);
	ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
	if(ret!= SQLITE_OK )
	{
		printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
	}


	sprintf(stream,"hik_p");
	param = get_http_param_value(http_request->URI,stream);		
	sprintf(sql,"update event set hik_p= \"%s\" where id = 1",param);
	ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
	if(ret!= SQLITE_OK )
	{
		printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
	}

	sprintf(stream,"hik_t");
	param = get_http_param_value(http_request->URI,stream);		
	sprintf(sql,"update event set hik_t= \"%s\" where id = 1",param);
	ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
	if(ret!= SQLITE_OK )
	{
		printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
	}

	sprintf(stream,"plc_addr");
	param = get_http_param_value(http_request->URI,stream);		
	if(verify_ip_address(param)){
		sprintf(sql,"update plc_info set address= \"%s\" where id = 1",param);
		ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
		if(ret!= SQLITE_OK )
		{
			printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}
	}


	sprintf(stream,"plc_port");
	param = get_http_param_value(http_request->URI,stream);		
	if(verify_num(param)&&(atoi(param)<20000)&&(atoi(param)>200)){
		sprintf(sql,"update plc_info set port= \"%s\" where id = 1",param);
		ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
		if(ret!= SQLITE_OK )
		{
			printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}
	}


	sprintf(stream,"r0");
	param = get_http_param_value(http_request->URI,stream);		
	if(verify_num(param)&&(atoi(param)<50000)){
		sprintf(sql,"update plc_info set r0= \"%s\" where id = 1",param);
		ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
		if(ret!= SQLITE_OK )
		{
			printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}
	}
	sprintf(stream,"r1");
	param = get_http_param_value(http_request->URI,stream);		
	if(verify_num(param)&&(atoi(param)<50000)){
		sprintf(sql,"update plc_info set r1= \"%s\" where id = 1",param);
		ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
		if(ret!= SQLITE_OK )
		{
			printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}
	}
	sprintf(stream,"protocol");
	param = get_http_param_value(http_request->URI,stream);		
	if(verify_num(param)&&(atoi(param)<10)){
		sprintf(sql,"update plc_info set protocol= \"%s\" where id = 1",param);
		ret= sqlite3_exec( db, sql, NULL, NULL, &errmsg );
		if(ret!= SQLITE_OK )
		{
			printf( "更新失败，错误码:%d，错误原因:%s\r\n", ret, errmsg );
		}
	}



	sqlite3_close( db );
	return 1;
	//else
	//	return 1;

}

void cgi_fileup_reboot(void)
{
	
}
//仅适用submit，ajax报文头缺boundary
uint8_t cgi_fileup_process(conn_t c,http_request_t http_request,uint8_t *http_buf,uint16_t len)
{
	FILE *fp = NULL;
	char boundary[64];
	char filename[64];
	char filePath[64];
	char sub[10];
	uint32_t content_len,upload_file_len,rx_len=0;
	uint8_t recv_on = 0;
	uint32_t counter=0;
	uint8_t bootsign[16]={0};
	char* pos0;
	char* pos1;
	char* pos2;
	char* pos3;
	char* pos4;	
	char* fff;
	char testPrint[1024];

	int i =0;
	int available=0;
	int ret=-1;

	uint8_t rcvBuf[6*1024*1024];
	uint32_t leaveCounter=0;
	
	pos0=http_request->URI;
	fff = pos0;
	*(fff+len-5)='\0';
	mid(pos0,"boundary=","\r\n",boundary);
	mid(pos0,"Content-Length: ","\r\n",sub);//含有标记的总大小
	printf("c_len: %s\r\n",sub);
	content_len=atoi(sub);//73k
	printf("len: %d\r\n",len);
	printf("boundary: %s\r\n",boundary);
	//第一包内容，特殊标记的后4个\r\n为正文
	pos1=strstr(pos0,boundary);
	pos1+=strlen(boundary);
	pos2=strstr(pos1,boundary);//特殊标记头位置,长度要多2个,--boundary
	if(pos2==NULL){
		printf("begin find \r\n");
		for(i=0;i<10;i++){
			usleep(10000);
			if(c->fd!= -1)
				ret = ioctl(c->fd, FIONREAD,&available);
			if (available>0&& ret == 0)		
			{
				ret =c->recv(c, (u_char*)(fff+len-5), 1024); 				
				printf("ret %d \r\n",ret);
				//tPrint(pos0);
				//tPrint(pos1);
				//tPrint(fff);
				len+=ret;
				//pos2=strstr(pos1,boundary);//特殊标记头位置,长度要多2个,--boundary
				pos2=str_nstr(pos1,boundary,ret);//特殊标记头位置,长度要多2个,--boundary
				break;
			}
		}

	}
	if(i==10) return 0;
	//printf(">>%s \r\n",pos0);
	mid(pos2,"filename=\"","\"",filename);
	printf(">>%s\r\n",filename);
	pos3=strstr(pos2,"\r\n\r\n")+4;//正文起点

	//找解释标志
	if((pos4=strstr(pos3,boundary))!=NULL)//第一包就结束
	{
		printf("only one package\r\n");
		upload_file_len=pos4-pos3-4;//正文结尾有\r\n--4个	
		return 0;
		//fuzhidaoneicun		
	}else
	{
		upload_file_len=content_len-(pos3-pos2+2)-2-(2+strlen(boundary)+2)-2;
		printf("4\r\n");
		if(upload_file_len>(6*1024*1024))
		{
			printf("huge file!\n");
			return 0;
		}			
		printf("this len %d %d\r\n",len,pos3-pos0);
		rx_len = len -(pos3-pos0)-5;//"POST "
		recv_on = 1;
		printf("@@@%d\r\n",rx_len);
		printf("2\r\n");
		memcpy(rcvBuf,pos3,rx_len);//取第一包中的正文
	//	printf(">>%s\r\n",rcvBuf);
		//fwrite(http_buf+(len-rx_len),1,rx_len,fp);
		for (counter = 0; counter < rx_len; counter++)
		{
			//*(volatile uint8_t*) (IAP_SRAM_FIRM_DATA_ADDR + counter) = *(http_buf+(len-rx_len)+counter);
//			printf("%02x",*(__IO uint8_t*) (SRAM_FIRM_ADDR + counter));
		}
	}
	printf("@@@FirmSize:%d\r\n",upload_file_len);	
	leaveCounter=0;
	while(recv_on&&leaveCounter<2000)
	{
		uint16_t temp_len=1024;
		uint16_t tail_len;
		 available=0;
		 ret=-1;
		//temp_len = getSn_RX_RSR(s);
		usleep(100);
		leaveCounter++;
		if(c->fd!= -1)
			ret = ioctl(c->fd, FIONREAD,&available);
		if (available>0&& ret == 0)		
		{
			temp_len = c->recv(c, (uint8_t*)http_buf, 2048); 				
//			*(((uint8_t*)http_buf)+temp_len) = 0;				//第二包
			//printf("\r\n@@收请求%d\r\n%s",temp_len,http_buf);
			tail_len=temp_len-4-strlen(boundary)-4-2;
			if(upload_file_len<=(rx_len+temp_len))
			{
				printf("tail!\r\n");//是尾部

				
				recv_on=0;
				memcpy(rcvBuf+rx_len,http_buf,upload_file_len-rx_len);
				if(strlen(filename)){
					sprintf(filePath,"update\/%s",filename);
					fp = fopen(filePath,"wb+");
					if(fp==NULL) {
						printf("open %s faild\r\n",filePath);
						return 1;
					}
					fwrite(rcvBuf,upload_file_len,1,fp);
					fclose(fp);
				}
				for (counter = 0; counter < tail_len; counter++)
				{
					//*(volatile uint8_t*) (IAP_SRAM_FIRM_DATA_ADDR + rx_len + counter) = *(http_buf + counter);
//					printf("%02x",*(__IO uint8_t*) (SRAM_FIRM_ADDR + rx_len+ counter));
				}
				for (counter = 0; counter < 16; counter++)
				{
					//bootsign[counter]=*(volatile uint8_t*) (IAP_SRAM_FIRM_DATA_ADDR + 0x5000 + counter);
				}
				printf("flag:%s\r\n",bootsign);
				if(strcmp((char*)bootsign,"Powered by Flame")!=0)
				{
					printf("wrong firm\n");
					return 1;
				}
					
			}else
			{
//				memcpy(SRAM_ADDR+rx_len,http_request,temp_len);
				for (counter = 0; counter < temp_len; counter++)
				{
					//*(volatile uint8_t*) (IAP_SRAM_FIRM_DATA_ADDR + rx_len + counter) = *(http_buf + counter);
				}
				memcpy(rcvBuf+rx_len,http_buf,temp_len);//取第一包中的正文
			//	printf(">>%s\r\n",rcvBuf+rx_len);
				rx_len+=temp_len;
				printf("rx_len=%d\r\n",rx_len);//第二包长度,中间包				
			}
		}
	}
	//*(volatile uint32_t*) (IAP_SRAM_FIRM_LEN_ADDR) = upload_file_len;

	return 1;
}
void make_upok_response(uint16_t delay, char* url,char* cgi_response_buf)
{
  sprintf(cgi_response_buf,"<html><head><title>Function configuration</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j + ' ';j--;setTimeout('func()',1000);if(j==0)location.href='http://%d.%d.%d.%d/';}</script></head><body onload='func()'>upload firm success, new firm system will reboot in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay,url[0],url[1],url[2],url[3]);
  return;
}
void make_uperror_response(uint16_t delay, char* url,char* cgi_response_buf)
{
  sprintf(cgi_response_buf,"<html><head><title>Function configuration</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j + ' ';j--;setTimeout('func()',1000);if(j==0)location.href='http://%d.%d.%d.%d/';}</script></head><body onload='func()'>Sorry, you upload wrong firmware, the module will reboot in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay,url[0],url[1],url[2],url[3]);
  return;
}
//***************************************************************/


 /**
*@brief		  转化转义字符为ascii charater 
*@param		  url：需要转化网页地址
*@return	  无
*/
void unescape_http_url(char * url)
{
	int x, y;
	for (x = 0, y = 0; url[y]; ++x, ++y) 
	{
		if ((url[x] = url[y]) == '%') 
		{
			url[x] = c2d(url[y+1])*0x10+c2d(url[y+2]);
			y+=2;
		}
	}
	url[x] = '\0';
}

 /**
*@brief		  执行一个答复，如 html, gif, jpeg,etc.
*@param		  buf- 答复数据
*@param			type- 答复数据类型
*@param			len-  答复数据长度
*@return	  无
*/
void make_http_response_head(u_char* buf,uint8_t type,uint32_t len)
{
	char * head;
	char tmp[10];
	memset(buf,0x00,MAX_URI_SIZE); 
	/* 文件类型*/
	if 	(type == PTYPE_HTML) head = RES_HTMLHEAD_OK;
	else if (type == PTYPE_GIF)	  head = RES_GIFHEAD_OK;
	else if (type == PTYPE_TEXT)	head = RES_TEXTHEAD_OK;
	else if (type == PTYPE_JPEG)	head = RES_JPEGHEAD_OK;
	else if (type == PTYPE_FLASH)	head = RES_FLASHHEAD_OK;
	else if (type == PTYPE_MPEG)	head = RES_MPEGHEAD_OK;
	else if (type == PTYPE_PDF)	  head = RES_PDFHEAD_OK;

	sprintf(tmp,"%d", len);	
	strcpy((char*)buf, head);
	strcat((char*)buf, tmp);
	strcat((char*)buf, "\r\n\r\n");
}

 /**
*@brief		  解析每一个http响应
*@param		  request： 新格式体，http发送的原始数据组
*@return	  无，strtok针对纯文本有效，处理bin不能使用新格式体
*/
int parse_http_request(http_request_t  request,u_char * buf,uint32_t len)
{
	char * nexttok=NULL;
	char tmp_buf[128];
	u_char *head ,*tail;
	if(request->headLen == 0){
		
		if(!strncmp(buf, "GET",3) ||!strncmp(buf, "get",3) )
		{
			request->METHOD = METHOD_GET;
			nexttok = buf+4;			
		}
		else if (!strncmp(buf, "HEAD",4) || !strncmp(buf,"head",4))	
		{
			request->METHOD = METHOD_HEAD;
			nexttok = buf+5;  		
		}
		else if (!strncmp(buf, "POST",4) || !strncmp(buf,"post",4))
		{
			request->METHOD = METHOD_POST;
			nexttok = buf+5;
		}
		else
		{
			request->METHOD = METHOD_ERR;
		}

		if(!nexttok)
		{
			request->METHOD = METHOD_ERR; 			
			return;
		}
		tail = str_nstr(buf,"\r\n\r\n",len);
		if(tail){
			request->headLen = tail-buf+4;
		}
		mid(buf,"Content-Length: ","\r\n",tmp_buf);
		if(strlen(tmp_buf)){
			request->contentLen=atoi16(tmp_buf,10);
		}
		//printf(">>%s",buf);
		printf(">>>>>>>contentLen=%d:%d\r\n",request->contentLen,request->headLen);
		memcpy(request->URI,buf,len); 					
	}

	request->pkgLen+=len;

	if(request->METHOD==METHOD_GET)
		return 0;	
	printf(">>>>>>>con=%d:%d:%d\r\n",request->contentLen,request->headLen,request->pkgLen);
	if(request->METHOD==METHOD_POST &&request->pkgLen== request->headLen+request->contentLen){
		memcpy(request->URI+strlen(request->URI),buf,len); 					
		return 0;
	}
	return 1;
}

 /**
*@brief		  得到响应过程中的下一个参数
*@param		  url：需要转化网页地址
*@param			param_name： 
*@return	  返回一个数据url中某参数名的=号之后的参数
*/
uint8_t* get_http_param_value(char* uri, char* param_name)
{
	uint16_t len;
	uint8_t* pos2;
	uint8_t* name=0; 
	uint8_t *ret=pub_buf;
	uint16_t content_len=0;
	char tmp_buf[10]={0x00,};
	if(!uri || !param_name)
		return 0;
	/***************/
	mid(uri,"Content-Length: ","\r\n",tmp_buf);
	content_len=atoi16(tmp_buf,10);
	uri = (char*)strstr(uri,"\r\n\r\n");
	uri+=4;
	//printf("uri=%s\r\n",uri);//ip=172.16.10.245&sub=255.255.255.0&gw=172.16.10.1
	uri[content_len]=0;
	/***************/	 
	name= (uint8_t *)str_nstr(uri,param_name,4096);
	if(name)
	{
		name += strlen(param_name) + 1; 
		pos2=(uint8_t*)strstr((char*)name,"&");
		if(!pos2) 
		{
			pos2=name+strlen((char*)name);
		}
		len=0;
		len = pos2-name;
		if(len)
		{
			ret[len]=0;
			strncpy((char*)ret,(char*)name,len);
			unescape_http_url((char *)ret);
			replacetochar((char *)ret,'+',' ');
		}
		else
			ret[0]=0;
	}
	else
		return 0;
	return ret;		
}
//端口，响应体，文件
void static_string_page_respond(conn_t c,u_char* http_response,char * h_file)
{
	uint32_t file_len=0;	
	uint32_t send_len=0;
	uint8_t readBuf[1024*16];
		printf("try to open %s\r\n",h_file);
	FILE *fp = fopen(h_file,"r");
	if(fp == NULL){
		printf("can not open %s\r\n",h_file);
		memcpy(http_response, ERROR_HTML_PAGE, sizeof(ERROR_HTML_PAGE));//404
		c->send(c, (uint8_t *)http_response, strlen((char const*)http_response));
		return;
	}
	file_len = fread(readBuf,1,sizeof(readBuf),fp);
	printf("file len %d \r\n",file_len);
	fclose(fp);
	make_http_response_head(http_response, PTYPE_HTML,file_len);
	c->send(c,http_response,strlen((char const*)http_response));//先发响应头
	send_len=0;
	while(file_len)
	{
		if(file_len>1024)
		{
			c->send(c, (uint8_t *)readBuf+send_len, 1024);
			send_len+=1024;
			file_len-=1024;
		}
		else
		{
			c->send(c, (uint8_t *)readBuf+send_len, file_len);
			send_len+=file_len;
			file_len-=file_len;
		} 
	}
}

/**
*@brief		接收http请求报文并发送http响应
*@param		s: http服务器socket
*@param		buf：解析报文内容
*@return	无
*/
//返回0表示解析成功了
int proc_http(conn_t c, uint8_t * buf,uint32_t len)
{
	char* name,*head; 											
	char req_name[128]={0x00,};							
	uint8_t* http_response;					/*定义一个http响应报文的指针*/
	int res =0;
	http_request_t http_request;			/*定义http请求报文头的结构体指针*/
	http_response = ((http_request_t)c->data)->tx_buf;
	http_request = (http_request_t)c->data;
	res = parse_http_request(http_request, buf,len);    		/*解析http请求报文头*/
	mid(http_request->URI, "/", " ", req_name);		/*获取该请求的文件名*/
	printf("post %s \r\n",req_name);
	if(res==1&&strcmp(req_name,"fileup.cgi")!=0) return 1;
	u_char tx_buf[4096];
				
	switch (http_request->METHOD)		
	{
		case METHOD_ERR :																			/*请求报文头错误*/
			memcpy(http_response, ERROR_REQUEST_PAGE, sizeof(ERROR_REQUEST_PAGE));
			c->send(c, (uint8_t *)http_response, strlen((char const*)http_response));
			break;		
		case METHOD_HEAD:																			/*HEAD请求方式*/
		case METHOD_GET:																			/*GET请求方式*/
			head = str_nstr(http_request->URI," ",128);
			if(head){
				name = str_nstr(head+1," ",128);
				if(name){
					memcpy(req_name,head+1,name-head-1);
				}
			}
			name = req_name;
			printf("get %s \r\n",name);
#ifdef USE_FATFS_FLASH
			snprintf(trans_c.filename, sizeof(trans_c.filename),"%s%s",WEB_ROOT_PATH,name);
			printf("http request f_path:%s\r\n",trans_c.filename);
#endif
			if(strcmp(name,"/index.htm")==0 || strcmp(name,"/")==0 || (strcmp(name,"/index.html")==0))
			{
				static_string_page_respond(c,http_response,INDEX_HTML);
			}
			else if(strcmp(name,"/post_config.js")==0)//获取解码器分屏的参数
			{
				make_post_config_setting_json_callback(tx_buf,json_config);
				sprintf((char *)http_response,"%s%d\r\n\r\n%s",RES_HTMLHEAD_OK,strlen(tx_buf),tx_buf);
				c->send(c, (uint8_t *)http_response, strlen((char const*)http_response));
			}	
			else if(strcmp(name,"/get_scene.js")==0)//
			{
				make_get_scene_json_callback(tx_buf,json_config);
				sprintf((char *)http_response,"%s%d\r\n\r\n%s",RES_HTMLHEAD_OK,strlen(tx_buf),tx_buf);
				c->send(c, (uint8_t *)http_response, strlen((char const*)http_response));
			}	
			else if(strcmp(name,"/scene")==0)//获取解码器分屏的参数
			{
				static_string_page_respond(c,http_response,SCENE_HTML);
			}	
			else
			{
				memcpy(http_response, ERROR_HTML_PAGE, sizeof(ERROR_HTML_PAGE));//404
				c->send(c, (uint8_t *)http_response, strlen((char const*)http_response));
			}
			break;

		case METHOD_POST:									/*POST请求*/
			mid(http_request->URI, "/", " ", req_name);		/*获取该请求的文件名*/
			printf("post %s \r\n",req_name);
			if(strcmp(req_name,"reboot.cgi")==0)			//post文件体				  	
			{
				sprintf((char *)http_response,"%s%d\r\n\r\n%s",RES_HTMLHEAD_OK,strlen(tx_buf),tx_buf);				
				c->send(c, (uint8_t *)http_response, strlen((char *)http_response));		/*发送http响应*/
				printf("reboot......\r\n");
				system("reboot");
				//disconnect(s);															/*断开socket连接*/				
				//reboot();//重启。。。
			}
			else if(strcmp(req_name,"postset.cgi")==0&&login_flag)			//post文件体				  	
			{
				cgi_postset(http_request);
				//make_cgi_response(8,(char*)ConfigMsg.lip,tx_buf);	/*生成响应的文本部分js*/        
				//sprintf((char *)http_response,"%s%d\r\n\r\n%s",RES_HTMLHEAD_OK,strlen(tx_buf),tx_buf);																												
				c->send(c, (uint8_t *)http_response, strlen((char *)http_response));		/*发送http响应*/
				//disconnect(s);															/*断开socket连接*/		
				//reboot();				
			}
			else if(strcmp(req_name,"test.cgi")==0)			//post文件体			index	  	
			{
				char temp;
				char msg[64];
				temp = cgi_test_process(http_request);
				if(temp)
				{
					//sprintf(tx_buf,"IN %c Send SUCCESS",temp);
					sprintf(msg,"%c",temp);
					//makePostJson(temp);//转交
				}else
				{
					//memcpy(tx_buf,"ERROR",5);
				}
				//sprintf((char *)http_response,"%s%d\r\n\r\n%s",RES_HTMLHEAD_OK,strlen(tx_buf),tx_buf);																												
				c->send(c, (uint8_t *)http_response, strlen((char *)http_response));		/*发送http响应*/		
			}
			else if(strcmp(req_name,"geshihua.cgi")==0)			  	//root
			{
				char temp;
				temp = cgi_geshihua_process(http_request);
				if(temp)
				{
					sprintf(tx_buf,"format success!");
				}else
				{
					sprintf(tx_buf,"format fail!");
				}
				sprintf((char *)http_response,"%s%d\r\n\r\n%s",RES_HTMLHEAD_OK,strlen(tx_buf),tx_buf);																												
				c->send(c, (uint8_t *)http_response, strlen((char *)http_response));		/*发送http响应*/			
			}
			else if(strcmp(req_name,"setScene.cgi")==0)			//post文件体				  	
			{
				char temp;
				temp = set_scene_process(http_request);
				if(temp)
				{
					sprintf(tx_buf,"set scene success!");
				}else
				{
					sprintf(tx_buf,"format fail!");
				}
				sprintf((char *)http_response,"%s%d\r\n\r\n%s",RES_HTMLHEAD_OK,strlen(tx_buf),tx_buf);																												
				c->send(c, (uint8_t *)http_response, strlen((char *)http_response));		/*发送http响应*/			
			}
			else if(strcmp(req_name,"login.cgi")==0)			//post文件体				  	
			{
				uint8_t * param;
				param = get_http_param_value(http_request->URI,"input");
				printf("++%s",(param));
				if(strcmp((char*)param,"123")==0)
				{
					sprintf(tx_buf,"0");
					login_flag = 1;
				}else
				{
					memcpy(tx_buf,"1",1);
				}
				sprintf((char *)http_response,"%s%d\r\n\r\n%s",RES_HTMLHEAD_OK,strlen(tx_buf),tx_buf);																												
				c->send(c, (uint8_t *)http_response, strlen((char *)http_response));		/*发送http响应*/	
			}
			else if(strcmp(req_name,"fileup.cgi")==0&&login_flag)			//post文件体			root	  	
			{
				char temp;
				temp = cgi_fileup_process(c,http_request,http_request->rx_buf,len);
				if(temp)
				{
					make_upok_response(15,(char*)json_config.localIp,tx_buf);	/*生成响应的文本部分js*/        
				}else
				{
					make_uperror_response(3,(char*)json_config.localIp,tx_buf);	/*生成响应的文本部分js*/     
				}			
				sprintf((char *)http_response,"%s%d\r\n\r\n%s",RES_HTMLHEAD_OK,strlen(tx_buf),tx_buf);																												
				c->send(c, (uint8_t *)http_response, strlen((char *)http_response));		/*发送http响应*/			
				if(temp)
				{
					cgi_fileup_reboot();	    
				}
			}
			break;
			
		default :
			break;
	}
	return 0;
}
void http_trans(conn_t c)
{
#ifdef USE_FATFS_FLASH	
	
#endif	
}
/**
*@brief		完成http响应
*@param		无
*@return	无
*/
int do_https(event_t ev)
{
	conn_t c = (conn_t)ev->data;
	uint16_t len;
	
	http_request_t http_request;										/*定义一个结构指针*/
	//http_request = (st_http_request*)rx_buf;					 
	/* http service start */
	/*socket处于连接状态*/
	//len = c->recv(c, (uint8_t*)http_request, len); 				/*接收http请求*/			
	//*(((uint8_t*)http_request)+len) = 0;
	//proc_http(c, (uint8_t*)http_request,len);					/*接收http请求并发送http响应*/			
	if(trans_c.transmit_task)
	{
		http_trans(c);			
	}else
	{
		//printf("s>>>13  0\r\n");
		//disconnect(ch);	//时间很长 要拆分
		//printf("s>>>13  1\r\n");
	}

	//准备关闭连接 看是否还有数据
	if (0)
	{
		//len = recv(ch, (uint8_t*)http_request, len);				/*接收http请求*/      
		*(((uint8_t*)http_request)+len) = 0;
		proc_http(c, (uint8_t*)http_request,len);					/*接收http请求并发送http响应*/
	}
	//disconnect(ch);		
}





