
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vdec.h"
#include "vo.h"
#include "vgs.h"
#include "decoder.h"
 
 
/* g_s32VBSource: 0 to module common vb, 1 to private vb, 2 to user vb
   And don't forget to set the value of VBSource file "load3535" */
FY_S32 g_s32VBSource = 0;
//VB_POOL g_ahVbPool[VB_MAX_POOLS] = {[0 ... (VB_MAX_POOLS-1)] = VB_INVALID_POOLID};

static VDEC_CHN_ATTR_S stVdecChnAttr[CHNS];

 
 int vdec_start_mux_voChn_ext(FY_U32	s32ChnNum, int enMode,  int video_width, int video_height, PAYLOAD_TYPE_E pt_types[],int fb_cnt, int bind,VO_LAYER VoLayer,VO_CHN startVoChn)
 {
    int	i;
	FY_S32 ret;
	SIZE_S astSize[CHNS];//
	VB_CONF_S stModVbConf;
	//PAYLOAD_TYPE_E pt_types[VDEC_MAX_CHN_NUM];
	//char* filenames[VDEC_MAX_CHN_NUM];
	VDEC_MOD_PARAM_S mod_parm;
	int bind_vo_channel;
    
    
	//这里只需要将前4个通道设置成1080P分辨率
	astSize[0].u32Width	= 3840;
	astSize[0].u32Height =2160;
    for(i = 1 ; i < 4; i++) {
    	astSize[i].u32Width	=1920;
	    astSize[i].u32Height =1080;
    }
    for(i = 4 ; i < s32ChnNum; i++) {
    	astSize[i].u32Width	= 960;//之前是704
	    astSize[i].u32Height = 576;
    }

	

	astSize[8].u32Width	= 960;
	astSize[8].u32Height = 576;
	

	astSize[10].u32Width	= 960;
	astSize[10].u32Height = 576;
	FY_MPI_VDEC_GetModParam(&mod_parm);
	if(mod_parm.u32VBSource==0)
	{
		vdec_mod_commPoolConf_ext(&stModVbConf,pt_types,astSize,s32ChnNum,fb_cnt);
		vdec_init_modCommVb(&stModVbConf);
	}
	bind_vo_channel = s32ChnNum;
    vdec_vo_init_layer(enMode,VoLayer,startVoChn,bind_vo_channel);

	vdec_vgs_init_layer(s32ChnNum,enMode,VoLayer);

	ret = vdec_vgs_bind_vo_layer(bind_vo_channel,VoLayer,startVoChn);
	if(ret) goto FAIL2;


	vdec_chnAttr(s32ChnNum, &stVdecChnAttr[0],	pt_types, astSize);
	for(i=0;i<CHNS;i++)
		stVdecChnAttr[i].stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;


	ret = vdec_start(s32ChnNum,stVdecChnAttr,fb_cnt);

	for(i=0;i<s32ChnNum;i++)
	{
		ret = vdec_bind_vgs((VDEC_CHN)i,(VGS_CHN)i);
		if(ret) goto FAIL2;
	}

	if(ret)
		goto FAIL2;


	FAIL2:
	 	return 0;

 }


 FY_VOID	vdec_mod_commPoolConf_ext(VB_CONF_S *pstModVbConf, PAYLOAD_TYPE_E *enTypes, SIZE_S *pstSize, FY_S32 s32ChnNum,FY_S32  fbCnt)
{
	int i;
	FY_S32 PicSize;
	PAYLOAD_TYPE_E enType;
	MPP_CHN_S chn;

	chn.enModId = FY_ID_VDEC;
	chn.s32DevId= 0;

	memset(pstModVbConf, 0,	sizeof(VB_CONF_S));
	pstModVbConf->u32MaxPoolCnt	= 1;
	for(i=0;i<s32ChnNum;i++)
	{
		chn.s32ChnId = i;
		enType = enTypes[i];
		VB_PIC_BLK_SIZE(pstSize[i].u32Width, pstSize[i].u32Height, enType, PicSize);
		pstModVbConf->astCommPool[i].u32BlkSize	= PicSize;
		pstModVbConf->astCommPool[i].u32BlkCnt	= fbCnt;
		FY_MPI_SYS_GetMemConf(&chn,pstModVbConf->astCommPool[i].acMmzName);
	}
	pstModVbConf->u32MaxPoolCnt	= s32ChnNum;
}



FY_S32	vdec_init_modCommVb(VB_CONF_S *pstModVbConf)
{
    FY_MPI_VB_ExitModCommPool(VB_UID_VDEC);

    if(0 == g_s32VBSource)
    {
        CHECK_RET(FY_MPI_VB_SetModPoolConf(VB_UID_VDEC, pstModVbConf), "FY_MPI_VB_SetModPoolConf");
        CHECK_RET(FY_MPI_VB_InitModCommPool(VB_UID_VDEC), "FY_MPI_VB_InitModCommPool");
    }

    return FY_SUCCESS;
}
 

 FY_S32 vdec_vo_init_layer(int enMode, VO_LAYER VoLayer, VO_CHN startVoChn, FY_S32 s32Chn)
{
	int i;
    FY_S32 s32Ret = FY_SUCCESS;

	FY_MPI_VO_SetAttrBegin(VoLayer);
	for(i=0;i<s32Chn;i++)
	{
		s32Ret = vo_start_chnOne(VoLayer, startVoChn + i, enMode);
		if(s32Ret != FY_SUCCESS)
			break;
	}
	FY_MPI_VO_SetAttrEnd(VoLayer);
	return s32Ret;
}


FY_S32 vdec_vo_deinit_layer(int enMode, VO_LAYER VoLayer,VO_CHN startVoChn, FY_S32 s32Chn)
{
	int i;
    FY_S32 s32Ret = FY_SUCCESS;

	for(i=0;i<s32Chn;i++)
	{
		s32Ret = vo_stop_chnOne(VoLayer, startVoChn + i, enMode);
		if(s32Ret != FY_SUCCESS)
			break;
	}
	return s32Ret;
}


FY_S32 vdec_vgs_bind_vo_layer(FY_S32 s32ChnNum, VO_LAYER VoLayer,VO_CHN startVoChn)
{
	int i;
	VGS_CHN VgsChn;
	VO_CHN VoChn;

	for(i=0;i<s32ChnNum;i++)
	{
		VgsChn = i;
		VoChn = startVoChn + i;

		CHECK_CHN_RET(vgs_bind_vo(VgsChn,VoLayer,VoChn),i,"SAMPLE_COMM_VGS_BindVo");
	}

	return FY_SUCCESS;
}

FY_S32 vdec_vgs_unbind_vo_layer(FY_S32 s32ChnNum, VO_LAYER VoLayer,VO_CHN startVoChn)
{
	int i;
	VGS_CHN VgsChn;
	VO_CHN VoChn;

	for(i=0;i<s32ChnNum;i++)
	{
		VgsChn = i;
		VoChn = startVoChn + i;

		CHECK_CHN_RET(vgs_unbind_vo(VgsChn,VoLayer,VoChn),i,"SAMPLE_COMM_VGS_UnBindVo");
	}

	return FY_SUCCESS;
}

FY_S32 vdec_bind_vgs(VDEC_CHN VdChn, VGS_CHN VgsChn)
{
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = FY_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;

    stDestChn.enModId = FY_ID_VGS;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VgsChn;

    CHECK_RET(FY_MPI_SYS_Bind(&stSrcChn, &stDestChn), "FY_MPI_SYS_Bind");

    return FY_SUCCESS;
}

FY_S32 vdec_unbind_vgs(VDEC_CHN VdChn, VGS_CHN VgsChn)
{
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = FY_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;

    stDestChn.enModId = FY_ID_VGS;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VgsChn;

    CHECK_RET(FY_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "FY_MPI_SYS_UnBind");

    return FY_SUCCESS;
}

FY_VOID	vdec_chnAttr(FY_S32 s32ChnNum,
	VDEC_CHN_ATTR_S	*pstVdecChnAttr, PAYLOAD_TYPE_E	enTypes[],	SIZE_S *pstSize)
{
	FY_S32 i;
	PAYLOAD_TYPE_E	enType;

    for(i=0; i<s32ChnNum; i++)
    {
		enType = enTypes[i];
        pstVdecChnAttr[i].enType       = enType;
		pstVdecChnAttr[i].u32BufSize   = 4 * pstSize[i].u32Width * pstSize[i].u32Height/5;
        pstVdecChnAttr[i].u32Priority  = 5;
        pstVdecChnAttr[i].u32PicWidth  = pstSize[i].u32Width;
        pstVdecChnAttr[i].u32PicHeight = pstSize[i].u32Height;
        if (PT_H264 == enType || PT_MP4VIDEO == enType)
        {
            pstVdecChnAttr[i].stVdecVideoAttr.enMode=VIDEO_MODE_FRAME;
            pstVdecChnAttr[i].stVdecVideoAttr.u32RefFrameNum = 2;
            pstVdecChnAttr[i].stVdecVideoAttr.bTemporalMvpEnable = 0;
        }
        else if (PT_JPEG == enType || PT_MJPEG == enType)
        {
            pstVdecChnAttr[i].stVdecJpegAttr.enMode = VIDEO_MODE_FRAME;
            pstVdecChnAttr[i].stVdecJpegAttr.enJpegFormat = JPG_COLOR_FMT_YCBCR420;
        }
        else if(PT_H265 == enType)
        {
            pstVdecChnAttr[i].stVdecVideoAttr.enMode= VIDEO_MODE_FRAME;
            pstVdecChnAttr[i].stVdecVideoAttr.u32RefFrameNum = 4;
            pstVdecChnAttr[i].stVdecVideoAttr.bTemporalMvpEnable = 1;
        }
    }
}



FY_S32 vdec_start(FY_S32 s32ChnNum, VDEC_CHN_ATTR_S *pstAttr, FY_U32 u32BlkCnt)
{
    FY_S32  i;
//    FY_U32 u32BlkCnt = 10;
//    VDEC_CHN_POOL_S stPool;

    for(i=0; i<s32ChnNum; i++)
    {
        //if(1 == g_s32VBSource)
        {
            CHECK_CHN_RET(FY_MPI_VDEC_SetChnVBCnt(i, u32BlkCnt), i, "FY_MPI_VDEC_SetChnVBCnt");
        }
        CHECK_CHN_RET(FY_MPI_VDEC_CreateChn(i, &pstAttr[i]), i, "FY_MPI_VDEC_CreateChn");
#if 0
        if (2 == g_s32VBSource)
        {
            stPool.hPicVbPool = g_ahVbPool[0];
            stPool.hPmvVbPool = -1;
            CHECK_CHN_RET(FY_MPI_VDEC_AttachVbPool(i, &stPool), i, "FY_MPI_VDEC_AttachVbPool");
        }
#endif
        CHECK_CHN_RET(FY_MPI_VDEC_StartRecvStream(i), i, "FY_MPI_VDEC_StartRecvStream");
        CHECK_CHN_RET(FY_MPI_VDEC_SetDisplayMode(i, VIDEO_DISPLAY_MODE_PLAYBACK), i, "FY_MPI_VDEC_SetDisplayMode");//设置的回放模式
    }

    return FY_SUCCESS;
}


FY_S32 vdec_stop(FY_S32 s32ChnNum)
{
    FY_S32 i;

    for(i=0; i<s32ChnNum; i++)
    {
        CHECK_CHN_RET(FY_MPI_VDEC_StopRecvStream(i), i, "FY_MPI_VDEC_StopRecvStream");
        CHECK_CHN_RET(FY_MPI_VDEC_DestroyChn(i), i, "FY_MPI_VDEC_DestroyChn");
    }

    return FY_SUCCESS;
}


FY_S32 vdec_load_userPic(char* fname, FY_U32 w,FY_U32 h, PIXEL_FORMAT_E format,VIDEO_FRAME_INFO_S* pUserFrame)
{
	FILE  *fp   = NULL;
	FY_U32 u32Size;
	VB_POOL pool_id = VB_INVALID_POOLID;
	VB_BLK  VbBlk = VB_INVALID_HANDLE;
	FY_U32 u32PhyAddr = 0;
	FY_U8 *pVirAddr = NULL;

	if(format != PIXEL_FORMAT_YUV_SEMIPLANAR_420 && format != PIXEL_FORMAT_YUV_SEMIPLANAR_422)
		return FY_FAILURE;

	fp = fopen(fname, "rb");
	if(NULL == fp) {
		printf("open file %s failed!\n",fname);
		return FY_FAILURE;
	}

	u32Size = w*h;
	if(format == PIXEL_FORMAT_YUV_SEMIPLANAR_420 )
		u32Size += u32Size/2;
	else
		u32Size += u32Size;

	pool_id = FY_MPI_VB_CreatePool(u32Size,1,NULL);
	if(pool_id == VB_INVALID_POOLID)
	{
		printf("FY_MPI_VB_CreatePool failed!\n");
		goto FAIL;
	}

	VbBlk = FY_MPI_VB_GetBlock(pool_id, u32Size,NULL);
	if(VbBlk == VB_INVALID_HANDLE)
	{
		printf("FY_MPI_VB_GetBlock failed!\n");
		goto FAIL;
	}

	u32PhyAddr = FY_MPI_VB_Handle2PhysAddr(VbBlk);
	pVirAddr = (FY_U8 *) FY_MPI_SYS_Mmap(u32PhyAddr, u32Size);
	if (NULL == pVirAddr)
	{
		printf("FY_MPI_SYS_Mmap failed!\n");
		goto FAIL;
	}
	fread(pVirAddr, 1, u32Size, fp);
	fclose(fp);

	memset(pUserFrame,0,sizeof(VIDEO_FRAME_INFO_S));
	pUserFrame->u32PoolId = pool_id;
	pUserFrame->stVFrame.u32Width = w;
	pUserFrame->stVFrame.u32Height = h;
	pUserFrame->stVFrame.u32Field = VIDEO_FIELD_FRAME;
	pUserFrame->stVFrame.enPixelFormat = format;
	pUserFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pUserFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pUserFrame->stVFrame.u32PhyAddr[0] = u32PhyAddr;
	pUserFrame->stVFrame.u32PhyAddr[1] = u32PhyAddr +  w*h;
	pUserFrame->stVFrame.pVirAddr[0] = pVirAddr;
	pUserFrame->stVFrame.pVirAddr[1] = pVirAddr +  w*h;
	pUserFrame->stVFrame.u32Stride[0] = w;
	pUserFrame->stVFrame.u32Stride[1] = format==PIXEL_FORMAT_YUV_SEMIPLANAR_420? (w/2) : w;
	pUserFrame->stVFrame.s16OffsetTop = 0;
	pUserFrame->stVFrame.s16OffsetBottom = 0;
	pUserFrame->stVFrame.s16OffsetLeft = 0;
	pUserFrame->stVFrame.s16OffsetRight = 0;
	pUserFrame->stVFrame.u32PrivateData = VbBlk;

	return FY_SUCCESS;

FAIL:

	if(VbBlk != VB_INVALID_HANDLE)
		FY_MPI_VB_ReleaseBlock(VbBlk);
	if(pool_id!= VB_INVALID_POOLID)
		FY_MPI_VB_DestroyPool(pool_id);
	if(fp!=NULL)
		fclose(fp);
	return FY_FAILURE;

}
