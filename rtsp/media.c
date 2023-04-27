#include "core.h"
#include "media.h" 

#include "connection.h"

extern poolList_t list;
rtspClient_t rc[16];

//返回 1 都是需要更新的
int set_cam_chn(int chn,netConfig_t netCfg)
{
	if(rc[chn]) {
		printf("%s:%s\n",rc[chn]->conn->peer_ip,netCfg->mediaInfo.camAddress);
		if(strcmp(rc[chn]->conn->peer_ip,netCfg->mediaInfo.camAddress)==0 && \
			strncmp((char *)rc[chn]->sess->url->data,netCfg->mediaInfo.camUrl,strlen(netCfg->mediaInfo.camUrl))==0){
			printf("ip url no change\n");
			return 0;
		}else{
			free_rtsp_clients(rc[chn]);
			printf("free rtsp client [%d]\r\n",chn);
			rc[chn] = NULL;
		}	
	}
	if(strlen(netCfg->mediaInfo.camAddress) == 0) return 1;
	if(strlen(netCfg->mediaInfo.camUrl) == 0) return 1;

	rc[chn] = init_rtsp_clients(list,netCfg->mediaInfo.camAddress,netCfg->mediaInfo.camPort,\
				   netCfg->mediaInfo.camUser,netCfg->mediaInfo.camPasswd,netCfg->mediaInfo.camUrl);
	rc[chn]->chn = chn;
	return 1;
}


//从多画面  切换到少画面时，会有遗留的rc继续接收流
//需要将其关掉

void free_old_subwin(int current,int old)
{
	int i;
	if(old == current) return;
	for(i=0;i<old;i++){
		if(rc[i])  free_rtsp_clients(rc[i]);
		rc[i] = NULL;
	}
}


void init_media()
{
	int i;
	for(i=0;i<16;i++) rc[i]=NULL;
	return;
}

void free_media(void)
{
	int i =0;
	for(i=0;i<16;i++){
		if(rc[i]) free_rtsp_clients(rc[i]);
		rc[i] =NULL;
	}
}
