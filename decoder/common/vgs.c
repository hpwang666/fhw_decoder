#include "vgs.h"
#include "vo.h"


FY_S32 vdec_vgs_init_layer(FY_S32 s32ChnNum, int enMode,VO_LAYER VoLayer)
{
	int i;
	VGS_CHN VgsChn;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_CHN_ATTR_S stChnAttr;
    FY_S32 s32Ret = FY_SUCCESS;
	VGS_CHN_PARA_S vgsMode = {
		.enChnMode = VGS_CHN_MODE_AUTO,//VGS_CHN_MODE_USER,
		.u32Width = 960,
		.u32Height = 1088,
		.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420,
	};

 	FY_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);

	for(i=0;i<s32ChnNum;i++)
	{
		VgsChn = i;

	    s32Ret = vo_get_chnRect(i, enMode, &stLayerAttr.stImageSize, &stChnAttr.stRect);
        if (s32Ret != FY_SUCCESS)
        {
            printf("failed with %#x!\n", s32Ret);
            return FY_FAILURE;
        }

        vgsMode.u32Width = ALIGN_BACK(stChnAttr.stRect.u32Width, 8);
        vgsMode.u32Height = ALIGN_BACK(stChnAttr.stRect.u32Height, 2);
		CHECK_CHN_RET(FY_MPI_VGS_CreateChn(VgsChn),i,"FY_MPI_VGS_CreateChn");
		CHECK_CHN_RET(FY_MPI_VGS_SetChnMode(VgsChn, 0, &vgsMode),i,"FY_MPI_VGS_SetChnMode");

	}
    return FY_SUCCESS;
}


FY_S32 vdec_vgs_reinit_layer(FY_S32 s32ChnNum, int enMode,VO_LAYER VoLayer)
{
	int i;
	VGS_CHN VgsChn;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_CHN_ATTR_S stChnAttr;
    FY_S32 s32Ret = FY_SUCCESS;
	VGS_CHN_PARA_S vgsMode = {
		.enChnMode = VGS_CHN_MODE_AUTO,//VGS_CHN_MODE_USER,
		.u32Width = 960,
		.u32Height = 1088,
		.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420,
	};

 	FY_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);

	for(i=0;i<s32ChnNum;i++)
	{
		VgsChn = i;

	    s32Ret = vo_get_chnRect(i, enMode, &stLayerAttr.stImageSize, &stChnAttr.stRect);
        if (s32Ret != FY_SUCCESS)
        {
            printf("failed with %#x!\n", s32Ret);
            return FY_FAILURE;
        }

        vgsMode.u32Width = ALIGN_BACK(stChnAttr.stRect.u32Width, 8);
        vgsMode.u32Height = ALIGN_BACK(stChnAttr.stRect.u32Height, 2);
		CHECK_CHN_RET(FY_MPI_VGS_SetChnMode(VgsChn, 0, &vgsMode),i,"FY_MPI_VGS_SetChnMode");

	}
    return FY_SUCCESS;
}

FY_S32 vdec_vgs_deinit(FY_S32 s32ChnNum)
{
	int i;
	VGS_CHN VgsChn;
	for(i=0;i<s32ChnNum;i++)
	{
		VgsChn = i;
	    CHECK_CHN_RET(FY_MPI_VGS_DestroyChn(VgsChn),i, "FY_MPI_VGS_DestroyChn");
	}

	return FY_SUCCESS;
}

FY_S32 vgs_bind_vo(VGS_CHN VgsChn,VO_LAYER VoLayer, VO_CHN VoChn)
{
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = FY_ID_VGS;
	if(VoChn>15)
    stSrcChn.s32DevId = 1;
	else
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VgsChn;

    stDestChn.enModId = FY_ID_VOU;
    stDestChn.s32DevId = VoLayer;
    stDestChn.s32ChnId = VoChn;

    CHECK_RET(FY_MPI_SYS_Bind(&stSrcChn, &stDestChn), "FY_MPI_SYS_Bind");

    return FY_SUCCESS;
}


FY_S32 vgs_unbind_vo(VGS_CHN VgsChn,VO_LAYER VoLayer, VO_CHN VoChn)
{
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = FY_ID_VGS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VgsChn;

    stDestChn.enModId = FY_ID_VOU;
    stDestChn.s32DevId = VoLayer;
    stDestChn.s32ChnId = VoChn;

    CHECK_RET(FY_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "FY_MPI_SYS_UnBind");

    return FY_SUCCESS;
}
