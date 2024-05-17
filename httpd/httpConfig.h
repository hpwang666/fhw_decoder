#ifndef _HTTP_CONFIG_H
#define _HTTP_CONFIG_H

#include "connet.h"
typedef struct _APP_FUNCTION_CFG											
{
	uint8_t token[2];
	uint8_t app_http_post;						
	uint8_t app_ftp_server;						
	uint8_t app_gps_time;
	uint8_t app_tmr;
	char  model[10];
}APP_FUNCTION_CFG;
extern APP_FUNCTION_CFG eeprom_app_config;



typedef struct _JSONPOST_CFG											
{
	uint8_t localIp[4];				/*解码器IP地址*/
	uint16_t dcport;		
	char user[10];
	char pass[16];	
	uint8_t sw[4];					/*in*/	
	uint8_t trigger[16][4];			/*触发的streamIP*/
}JSONPOST_CFG;



#define EEPROM_JSON_SET_ADDR 	160		//有rom起始地址检验
#define EEPROM_JSON_ADDR_CHECK1	0X1C
#define EEPROM_JSON_ADDR_CHECK2	0X6E

typedef struct _JSONEEPROM_CFG											
{
	uint8_t token[2];
	uint8_t localIp[4];				/*解码器IP地址*/
	uint8_t dcport[2];
	char user[10];
	char pass[16];				
	uint8_t sw[4];					/*in  代表4个切换输入*/	
	uint8_t trigger[16][4];			/*触发的streamIP*/
}JSONEEPROM_CFG;


#endif
