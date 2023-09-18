#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <time.h>
#include <sys/socket.h>
#include <fcntl.h>  

#include "decoder.h"



decEnv_t create_dec_chns(void)
{
	int i=0;
	FY_S32 s32Ret;
	decEnv_t decEnv = (decEnv_t)calloc(1,sizeof(struct decEnv_st));
	decEnv->dec25= (decoder_t)calloc(CHNS,sizeof(struct decoder_st));

	VIDEO_FRAME_INFO_S VdecUserPic;
	vdec_load_userPic("wsp.yuv",960,540,PIXEL_FORMAT_YUV_SEMIPLANAR_420,&VdecUserPic);

	for(i=0;i<CHNS;i++)
	{
		if(i<4)
			decEnv->dec25[i].buf = dec_buf_new(4*1024*1024);
		else
			decEnv->dec25[i].buf = dec_buf_new(512*1024);
		decEnv->dec25[i].PKG_STARTED = 0;

		s32Ret =FY_MPI_VDEC_SetUserPic((VDEC_CHN)i,&VdecUserPic);
		if(s32Ret != FY_SUCCESS)
		{	
			printf("FY_MPI_VDEC_SetUserPic fail for %#x!\n", s32Ret);
		}
	}
	return decEnv;
}

void free_dec_chns(decEnv_t decEnv)
{
	int i;
	for(i=0;i<CHNS;i++){
		dec_buf_free(decEnv->dec25[i].buf);
	}
}
