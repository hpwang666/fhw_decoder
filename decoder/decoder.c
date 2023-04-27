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
#define CHNS (16)



decEnv_t create_dec_chns(void)
{
	FY_S32 s32Ret;
	int i=0;
	decEnv_t decEnv = (decEnv_t)calloc(1,sizeof(struct decEnv_st));
	decEnv->dec25= (decoder_t)calloc(VDEC_CHN_NUM_25,sizeof(struct decoder_st));

	
	VIDEO_FRAME_INFO_S VdecUserPic;
	vdec_load_userPic("wsp.yuv",960,540,PIXEL_FORMAT_YUV_SEMIPLANAR_420,&VdecUserPic);


	for(i=0;i<VDEC_CHN_NUM_4;i++)
	{
		decEnv->dec25[i].buf = dec_buf_new(4*1024*1024);
		//decEnv->dec25[i].rtpPkgList = (rtpPkg_t)calloc(PKG_LIST_LEN,sizeof( struct rtpPkg_st));
		decEnv->dec25[i].PKG_STARTED = 0;
		//pthread_mutex_init(&(decEnv->dec25[i].decLock),NULL);

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
	for(i=0;i<VDEC_CHN_NUM_4;i++){
		dec_buf_free(decEnv->dec25[i].buf);
		//free(decEnv->dec25[i].rtpPkgList);
		//pthread_mutex_destroy(&(decEnv->dec25[i].decLock));
	}
}
