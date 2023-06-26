
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>


#include "connet.h"
#include "vo.h"
#include "vgs.h"
#include "decoder.h"
#include "vdec.h"
#include "osal.h"
#include "stream.h"

		
//当是rtp数据包时 就是传的数据否则就是状态
				//ring_push(ring, rc->chn,buf->head,rtpPkgLen);
				// ring_push(ring,CFG_CHNERR,(u_char *)&chnStatus,sizeof(struct chnStatus_st));
void pkgProcess(u_char *rtpPkg)
{
	FY_S32 s32Ret;
	int i;
	rtspChnStatus_t rtspChnStatus;
	static uint32_t u32WndNum=4;
	SAMPLE_VO_MODE_E layerMode = VO_MODE_4MUX;
	rtpPkg_t pkg = (rtpPkg_t)rtpPkg;
	if(pkg->cmd == 88){//切屏信号
		rtspChnStatus = (rtspChnStatus_t)pkg->data;
		//如果是四分屏，就会接收到4次切屏信号
		if(rtspChnStatus->subwin == u32WndNum){//窗口不需要重新分割
			if(rtspChnStatus->update){

				if(decEnv->dec25[rtspChnStatus->id].err){
					decEnv->dec25[rtspChnStatus->id].err = 0;
				}							
				dec_buf_init(decEnv->dec25[rtspChnStatus->id].buf);
				decEnv->dec25[rtspChnStatus->id].EN_PPS=0;
				decEnv->dec25[rtspChnStatus->id].EN_SPS=0;
				decEnv->dec25[rtspChnStatus->id].PKG_STARTED=0;
				decEnv->dec25[rtspChnStatus->id].refused = 0;
				decEnv->dec25[rtspChnStatus->id].waitIfream = 0;
				decEnv->dec25[rtspChnStatus->id].startRcv= 0;

				s32Ret =FY_MPI_VDEC_StopRecvStream(rtspChnStatus->id);
				if(s32Ret != FY_SUCCESS)
				{		
					printf("FY_MPI_VDEC_StopRecvStream fail for %#x!\n", s32Ret);
				}
				s32Ret =FY_MPI_VDEC_ResetChn(rtspChnStatus->id);
				if(s32Ret != FY_SUCCESS)
				{		
					printf("FY_MPI_VDEC_ResetChn fail for %#x!\n", s32Ret);
				}

#if 0
				//清除屏幕上的缓存数据
				s32Ret = HI_MPI_VO_ClearChnBuffer(VO_LAYER_VHD0, rtspChnStatus->id, HI_TRUE);
				if (HI_SUCCESS != s32Ret)
				{
					printf("pause vo chn failed! \n");
				}

				osalStartTimerEx(1,(0x00000001<<rtspChnStatus->id),300);//所有通道都加入定时清屏序列
#endif
			}
		}
		else{//因为分屏原因，这里只会处理 0  通道
			/* for(i=0; i<16; i++)
			   {
			   s32Ret = vout_unbind_vpss(VO_LAYER_VHD0, i, i, VPSS_CHN0);
			   if(s32Ret != HI_SUCCESS)
			   {	    
			   printf("vpss unbind vo fail for %#x!\n", s32Ret);
			//goto END6;
			}	
			} */
			//通道没有显示，只要不发视频流，就不会有问题。
			//比如1通道下，继续发4通道的数据，会导致解码器 a005800f 缓存溢出错误

			printf("cut from %d to %d \r\n",u32WndNum,rtspChnStatus->subwin);

			for(i=0;i<u32WndNum;i++){
				s32Ret =FY_MPI_VDEC_StopRecvStream(i);
				if(s32Ret != FY_SUCCESS)
				{	    
					printf("FY_MPI_VDEC_StopRecvStream for %#x!\n", s32Ret);
					//goto END6;
				}
				if(decEnv->dec25[i].err){
					decEnv->dec25[i].err = 0;
				}		
				FY_MPI_VDEC_ResetChn(i);
				dec_buf_init(decEnv->dec25[i].buf);
				decEnv->dec25[i].EN_PPS=0;
				decEnv->dec25[i].EN_SPS=0;
				decEnv->dec25[i].PKG_STARTED=0;
				decEnv->dec25[i].refused = 0;
				decEnv->dec25[i].waitIfream = 0;
				decEnv->dec25[i].startRcv= 0;

#if 0
				s32Ret = FY_MPI_VO_ChnBuffer(FY_VO_LAYER_VHD0, i, FY_TRUE);
				if (FY_SUCCESS != s32Ret)
				{
					printf("pause vo chn failed! \n");

				}
				osalClearTaskEvent(1,0x00000001<<i);
#endif
			}

			switch (u32WndNum)
			{
				case 1:layerMode=VO_MODE_1MUX;	break;
				case 2:layerMode=VO_MODE_1L_1R; break;
				case 4:layerMode=VO_MODE_4MUX;	break;
				case 6:layerMode=VO_MODE_1B_5S;	break; 
				case 9:layerMode=VO_MODE_9MUX;	break;
				case 16:layerMode=VO_MODE_16MUX;break;  
				default:
						printf("failed with layerMode\n");
						layerMode=VO_MODE_4MUX;
			}
			for(i=0;i<CHNS;i++)
			{
				s32Ret = vdec_unbind_vgs((VDEC_CHN)i,(VGS_CHN)i);
				//if(ret) goto FAIL2;
			}

			s32Ret=vdec_vgs_unbind_vo_layer(u32WndNum, FY_VO_LAYER_VHD0,0);
			if(s32Ret != FY_SUCCESS)
			{		
				printf("vdec_vgs_unbind_vo_layer for %#x!\n", s32Ret);
				//goto END4_4;
			}
			vdec_vgs_deinit(CHNS);
			vdec_vo_deinit_layer(layerMode, FY_VO_LAYER_VHD0, 0, CHNS);

			u32WndNum=rtspChnStatus->subwin;
			switch (rtspChnStatus->subwin)
			{
				case 1:layerMode=VO_MODE_1MUX;	break;
				case 2:layerMode=VO_MODE_1L_1R; break;
				case 4:layerMode=VO_MODE_4MUX;	break;
				case 6:layerMode=VO_MODE_1B_5S;	break; 
				case 9:layerMode=VO_MODE_9MUX;	break;
				case 16:layerMode=VO_MODE_16MUX;break;  
				default:
						printf("failed with layerMode\n");
						layerMode=VO_MODE_4MUX;
			}
			s32Ret = vdec_vo_init_layer(layerMode, FY_VO_LAYER_VHD0, 0, CHNS);
			if(s32Ret != FY_SUCCESS)
			{		
				printf("vdec start chn fail for %#x!\n", s32Ret);
				//goto END4_4;
			}

			vdec_vgs_init_layer(CHNS, layerMode,FY_VO_LAYER_VHD0);
			s32Ret= vdec_vgs_bind_vo_layer(rtspChnStatus->subwin,FY_VO_LAYER_VHD0,0);
			if(s32Ret != FY_SUCCESS)
			{		
				printf("vdec_vgs_bind_vo_layer for %#x!\n", s32Ret);
				//goto END4_4;
			}
			for(i=0;i<CHNS;i++)
			{
				s32Ret = vdec_bind_vgs((VDEC_CHN)i,(VGS_CHN)i);
				//if(ret) goto FAIL2;
			}
			osalStartTimerEx(1,(0x00000001<<0),300);
		}
	}
	else if(pkg->cmd == 89){//通道  错误 数据
		rtspChnStatus = (rtspChnStatus_t)pkg->data;

		if(decEnv->dec25[rtspChnStatus->id].err == 0){//
			printf("%d :net disconnected \n",rtspChnStatus->id);
			osalClearTaskEvent(1,0x00000001<<rtspChnStatus->id);
			s32Ret =FY_MPI_VDEC_StopRecvStream(rtspChnStatus->id);
			if(s32Ret != FY_SUCCESS)
			{	
				printf("FY_MPI_VDEC_StopRecvStream fail for %#x!\n", s32Ret);
			}
			s32Ret =FY_MPI_VDEC_ResetChn(rtspChnStatus->id);
			if(s32Ret != FY_SUCCESS)
			{	
				printf("FY_MPI_VDEC_ResetChn fail for %#x!\n", s32Ret);
			}

			dec_buf_init(decEnv->dec25[rtspChnStatus->id].buf);
			decEnv->dec25[rtspChnStatus->id].EN_PPS=0;
			decEnv->dec25[rtspChnStatus->id].EN_SPS=0;
			decEnv->dec25[rtspChnStatus->id].PKG_STARTED=0;
			decEnv->dec25[rtspChnStatus->id].refused = 0;
			decEnv->dec25[rtspChnStatus->id].waitIfream = 0;
			decEnv->dec25[rtspChnStatus->id].startRcv= 0;
			decEnv->dec25[rtspChnStatus->id].err = rtspChnStatus->err;
		}

	}	
	else {

		if(decEnv->dec25[pkg->cmd].err){//之前还有“无视频”标志

			printf("%d :net connected \n",pkg->cmd);
			decEnv->dec25[pkg->cmd].err = 0;

		}
		if(!decEnv->dec25[pkg->cmd].startRcv){
			
			s32Ret =FY_MPI_VDEC_StartRecvStream(pkg->cmd);
			if(s32Ret != FY_SUCCESS)
			{	
				printf("FY_MPI_VDEC_StartRecvStream fail for %#x!\n", s32Ret);
			}
			else{
				decEnv->dec25[pkg->cmd].startRcv=1;
			}
		}
		osalClearTaskEvent(1,0x00000001<<pkg->cmd);
	//	if(pkg->cmd < layerMode)//防止通道越界 允许解码
		if(pkg->cmd < u32WndNum)//TODO 这里要和 1 4 6 9 一致  防止通道越界 允许解码
			//printf("%d %d\r\n",pkg->cmd,pkg->len);
			process_rtp(pkg->data,pkg->len,pkg->cmd,&decEnv->dec25[pkg->cmd]);

	}
}

