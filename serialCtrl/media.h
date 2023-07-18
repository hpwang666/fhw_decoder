#ifndef _MEDIA_H
#define _MEDIA_H



#if 0
	0.每次收到切屏数据包，进行比对
	1.是否和当前屏幕分屏一致，不一致就下发切屏
	2.进行cam查找，找到了就将其修改通道号，修改rc的值
	3.找不到就新建rtspclient，并赋值给对应的rc
#endif

typedef struct netConfig_st  *netConfig_t;


#define PKG_MAGIC  (0x123456)

struct mediaInfo_st{
	
	int 	camPort;
	int		res;
	char    camAddress[64];
	char    camUrl[128];
	char	camUser[32];  
	char   	camPasswd[32];
	
};

struct netConfig_st{
	int     magic;
	int chn;
	int subwin; //1 4 6 9 16
	struct mediaInfo_st mediaInfo;//
};


#endif
