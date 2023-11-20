#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vo.h"


FY_S32 vo_init(FY_BOOL bIsHD, VO_INTF_SYNC_E enIntf)
{
    FY_S32 s32Ret = FY_SUCCESS;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    VO_PUB_ATTR_S stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;

    if (bIsHD) {
        printf("start DHD0.");
        VoDev = FY_VO_DEV_DHD0;
        VoLayer = FY_VO_LAYER_VHD0;

        stVoPubAttr.enIntfSync = enIntf;
		
		stVoPubAttr.enIntfType = VO_INTF_VGA|VO_INTF_HDMI;
		
        stVoPubAttr.u32BgColor = 0; //0x00FFFFFF
        
        s32Ret = FY_MPI_VO_SetPubAttr(VoDev, &stVoPubAttr);
        if (s32Ret != FY_SUCCESS)
        {
            printf("failed with %#x!\n", s32Ret);
            return FY_FAILURE;
        }

        s32Ret = FY_MPI_VO_Enable(VoDev);
        if (s32Ret != FY_SUCCESS)
        {
            printf("failed with %#x!\n", s32Ret);
            return FY_FAILURE;
        }

        memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
        s32Ret = vo_getWH(stVoPubAttr.enIntfSync, \
            &stLayerAttr.stImageSize.u32Width, \
            &stLayerAttr.stImageSize.u32Height, \
            &stLayerAttr.u32DispFrmRt);

         if (s32Ret != FY_SUCCESS)
        {
            printf("fy_vo_getWH failed with %#x!\n", s32Ret);
            return FY_FAILURE;
        }

        stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_422;
        stLayerAttr.stDispRect.s32X       = 0;
        stLayerAttr.stDispRect.s32Y       = 0;
        stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
        stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;

     
        printf("start single VHD0.");
        s32Ret = vo_start_singleLayer(VoLayer, &stLayerAttr, FY_TRUE);
        if (s32Ret != FY_SUCCESS){
            printf("SAMPLE_COMM_VO_StartSingleLayer failed!");
            return FY_FAILURE;
        } 
        

		if (false) {			//ENABLE_PIP
			printf("start PIP.");
			VoDev = FY_VO_DEV_DHD0;
       		VoLayer = FY_VO_LAYER_VPIP;

			stLayerAttr.stDispRect.s32X 	  = 200;
			stLayerAttr.stDispRect.s32Y 	  = 200;
			stLayerAttr.stDispRect.u32Width   = 320;
        	stLayerAttr.stDispRect.u32Height  = 240;

			stLayerAttr.stImageSize.u32Width  = 320;
			stLayerAttr.stImageSize.u32Height  = 320;

            //SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
			//SAMPLE_COMM_VO_StartChn(VoLayer, VO_MODE_1MUX);
		}
		

		//SAMPLE_COMM_AUDIO_StartHdmi();
    } else {
        printf("start DSD0.");
        VoDev = FY_VO_DEV_DSD0;
        VoLayer = FY_VO_LAYER_VSD0;

    }

    return s32Ret;
}

FY_S32 vo_deinit(FY_BOOL bIsHD)
{
    FY_S32 s32Ret = FY_SUCCESS;
    VO_DEV VoDev;
    VO_LAYER VoLayer;

    
    printf("stop VHD0.");
    VoLayer = FY_VO_LAYER_VHD0;
    s32Ret = FY_MPI_VO_DisableVideoLayer(VoLayer);
    if (s32Ret != FY_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
        return FY_FAILURE;
    }

    printf("stop DHD0.");
    VoDev = FY_VO_DEV_DHD0;
    
    s32Ret = FY_MPI_VO_Disable(VoDev);
    if (s32Ret != FY_SUCCESS)
    {
        printf("VoDev:%d,failed with %#x!\n", VoDev,s32Ret);
        return FY_FAILURE;
    }
    

    return s32Ret;
}

FY_S32 vo_getWH(VO_INTF_SYNC_E enIntfSync, FY_U32 *pu32W,FY_U32 *pu32H, FY_U32 *pu32Frm)
{
    switch (enIntfSync)
    {
        case VO_OUTPUT_960H_NTSC :   *pu32W = 480;  *pu32H = 960;  *pu32Frm = 60; break;
        case VO_OUTPUT_PAL       :   *pu32W = 720;  *pu32H = 576;  *pu32Frm = 25; break;
        case VO_OUTPUT_NTSC      :   *pu32W = 720;  *pu32H = 480;  *pu32Frm = 30; break;
        case VO_OUTPUT_576P50    :   *pu32W = 720;  *pu32H = 576;  *pu32Frm = 50; break;
        case VO_OUTPUT_480P60    :   *pu32W = 720;  *pu32H = 480;  *pu32Frm = 60; break;
        case VO_OUTPUT_800x480_60:   *pu32W = 800;  *pu32H = 480;  *pu32Frm = 60; break;
        case VO_OUTPUT_800x600_60:   *pu32W = 800;  *pu32H = 600;  *pu32Frm = 60; break;
        case VO_OUTPUT_800x1280_60:  *pu32W = 800;  *pu32H = 1280; *pu32Frm = 60; break;
        case VO_OUTPUT_720P25    :   *pu32W = 1280; *pu32H = 720;  *pu32Frm = 25; break;
        case VO_OUTPUT_720P30    :   *pu32W = 1280; *pu32H = 720;  *pu32Frm = 30; break;
        case VO_OUTPUT_720P50    :   *pu32W = 1280; *pu32H = 720;  *pu32Frm = 50; break;
        case VO_OUTPUT_720P60    :   *pu32W = 1280; *pu32H = 720;  *pu32Frm = 60; break;
        case VO_OUTPUT_1080I50   :   *pu32W = 1920; *pu32H = 1080; *pu32Frm = 25; break;
        case VO_OUTPUT_1080I60   :   *pu32W = 1920; *pu32H = 1080; *pu32Frm = 30; break;
        case VO_OUTPUT_1080P24   :   *pu32W = 1920; *pu32H = 1080; *pu32Frm = 24; break;
        case VO_OUTPUT_1080P25   :   *pu32W = 1920; *pu32H = 1080; *pu32Frm = 25; break;
        case VO_OUTPUT_1080P30   :   *pu32W = 1920; *pu32H = 1080; *pu32Frm = 30; break;
        case VO_OUTPUT_1080P50   :   *pu32W = 1920; *pu32H = 1080; *pu32Frm = 50; break;
        case VO_OUTPUT_1080P60   :   *pu32W = 1920; *pu32H = 1080; *pu32Frm = 60; break;
        case VO_OUTPUT_1024x600_60:  *pu32W = 1024; *pu32H = 600;  *pu32Frm = 60; break;
        case VO_OUTPUT_1024x768_60:  *pu32W = 1024; *pu32H = 768;  *pu32Frm = 60; break;
        case VO_OUTPUT_1280x1024_60: *pu32W = 1280; *pu32H = 1024; *pu32Frm = 60; break;
        case VO_OUTPUT_1366x768_60:  *pu32W = 1366; *pu32H = 768;  *pu32Frm = 60; break;
        case VO_OUTPUT_1440x900_60:  *pu32W = 1440; *pu32H = 900;  *pu32Frm = 60; break;
        case VO_OUTPUT_1280x800_60:  *pu32W = 1280; *pu32H = 800;  *pu32Frm = 60; break;
        case VO_OUTPUT_1600x1200_60: *pu32W = 1600; *pu32H = 1200; *pu32Frm = 60; break;
        case VO_OUTPUT_1680x1050_60: *pu32W = 1680; *pu32H = 1050; *pu32Frm = 60; break;
        case VO_OUTPUT_1920x1200_60: *pu32W = 1920; *pu32H = 1200; *pu32Frm = 60; break;
        case VO_OUTPUT_2560x1440_30: *pu32W = 2560; *pu32H = 1440; *pu32Frm = 30; break;
        case VO_OUTPUT_2560x1440_60: *pu32W = 2560; *pu32H = 1440; *pu32Frm = 60; break;
        case VO_OUTPUT_3840x2160_30: *pu32W = 3840; *pu32H = 2160; *pu32Frm = 30; break;
        case VO_OUTPUT_3840x2160_60: *pu32W = 3840; *pu32H = 2160; *pu32Frm = 60; break;
        case VO_OUTPUT_4096x2160_30: *pu32W = 4096; *pu32H = 2160; *pu32Frm = 30; break;
        case VO_OUTPUT_640x480_60:   *pu32W = 640;  *pu32H = 480;  *pu32Frm = 60; break;
        case VO_OUTPUT_240x320_60:   *pu32W = 240;  *pu32H = 320;  *pu32Frm = 60; break;
        case VO_OUTPUT_USER    :     *pu32W = 240;  *pu32H = 320;  *pu32Frm = 30; break;
        default:
            printf("vo enIntfSync not support!\n");
            return FY_FAILURE;
    }
    return FY_SUCCESS;
}


FY_S32 vo_start_singleLayer(VO_LAYER VoLayer,const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr, FY_BOOL isSingle)
{
    FY_S32 s32Ret = FY_SUCCESS;
    VO_PART_MODE_E enPartMode = VO_PART_MODE_SINGLE;
    VO_COMPRESS_ATTR_S stCompressAttr = {.bSupportCompress = FY_TRUE,};

    s32Ret = FY_MPI_VO_SetVideoLayerAttr(VoLayer, pstLayerAttr);
    if (s32Ret != FY_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
        return FY_FAILURE;
    }

    s32Ret = FY_MPI_VO_SetVideoLayerPartitionMode(VoLayer, enPartMode);
    if (s32Ret != FY_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
        return FY_FAILURE;
    }

    s32Ret = FY_MPI_VO_SetVideoLayerCompressAttr(VoLayer, &stCompressAttr);
    if (s32Ret != FY_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
        return FY_FAILURE;
    }

    s32Ret = FY_MPI_VO_EnableVideoLayer(VoLayer);
    if (s32Ret != FY_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
        return FY_FAILURE;
    }

    return s32Ret;
}



FY_S32 vo_start_chnOne(VO_LAYER VoLayer, VO_CHN voChn, SAMPLE_VO_MODE_E enMode)
{
    FY_S32 s32Ret = FY_SUCCESS;
    FY_U32 u32WndNum = 0;
    VO_CHN_ATTR_S stChnAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    FY_S32 s32ChnFrmRate;

    switch (enMode)
    {
        case VO_MODE_1MUX:
            u32WndNum = 1;
            break;
        case VO_MODE_4MUX:
            u32WndNum = 4;
            break;
        case VO_MODE_9MUX:
            u32WndNum = 9;
            break;
        case VO_MODE_16MUX:
            u32WndNum = 16;
            break;
        case VO_MODE_1B_5S:
            u32WndNum = 6;
            break;
        case VO_MODE_1B_7S:
            u32WndNum = 8;
            break;
        case VO_MODE_1L_1R:
            u32WndNum = 2;
            break;
        case VO_MODE_25MUX:
            u32WndNum = 25;
            break;
        case VO_MODE_36MUX:
            u32WndNum = 36;
            break;
        case VO_MODE_49MUX:
            u32WndNum = 49;
            break;
        case VO_MODE_64MUX:
            u32WndNum = 64;
            break;
        default:
            printf("failed with %#x!\n", s32Ret);
            return FY_FAILURE;
    }

    s32Ret = FY_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != FY_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
        return FY_FAILURE;
    }

    if (stLayerAttr.u32DispFrmRt <= 0)
    {
        s32ChnFrmRate = 30;
    }
    else if (stLayerAttr.u32DispFrmRt > 30)
    {
        s32ChnFrmRate = stLayerAttr.u32DispFrmRt / 2;
    }
    else
    {
        s32ChnFrmRate = stLayerAttr.u32DispFrmRt;
    }

    if (voChn < u32WndNum)
    {
        s32Ret = vo_get_chnRect(voChn, enMode, &stLayerAttr.stImageSize, &stChnAttr.stRect);
        if (s32Ret != FY_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return FY_FAILURE;
        }

        stChnAttr.u32Priority       = 0;
        stChnAttr.bDeflicker        = FY_FALSE;

        s32Ret = FY_MPI_VO_SetChnAttr(VoLayer, voChn, &stChnAttr);
        if (s32Ret != FY_SUCCESS)
        {
            printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
            return FY_FAILURE;
        }

        s32Ret = FY_MPI_VO_SetChnFrameRate(VoLayer, voChn, s32ChnFrmRate);
        if (FY_SUCCESS != s32Ret)
        {
            printf("failed with %#x!\n", s32Ret);
            return FY_FAILURE;
        }

        s32Ret = FY_MPI_VO_EnableChn(VoLayer, voChn);
        if (s32Ret != FY_SUCCESS)
        {
            printf("failed with %#x!\n", s32Ret);
            return FY_FAILURE;
        }
    }
    return FY_SUCCESS;
}

FY_S32 vo_stop_chnOne(VO_LAYER VoLayer, VO_CHN voChn, SAMPLE_VO_MODE_E enMode)
{
    FY_S32 s32Ret = FY_SUCCESS;
    FY_U32 u32WndNum = 0;

    switch (enMode)
    {
        case VO_MODE_1MUX:
            u32WndNum = 1;
            break;
        case VO_MODE_4MUX:
            u32WndNum = 4;
            break;
        case VO_MODE_9MUX:
            u32WndNum = 9;
            break;
        case VO_MODE_16MUX:
            u32WndNum = 16;
            break;
        case VO_MODE_1B_5S:
            u32WndNum = 6;
            break;
        case VO_MODE_1B_7S:
            u32WndNum = 8;
            break;
        case VO_MODE_1L_1R:
            u32WndNum = 2;
            break;
        case VO_MODE_25MUX:
            u32WndNum = 25;
            break;
        case VO_MODE_36MUX:
            u32WndNum = 36;
            break;
        case VO_MODE_49MUX:
            u32WndNum = 49;
            break;
        case VO_MODE_64MUX:
            u32WndNum = 64;
            break;
        default:
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return FY_FAILURE;
    }

    if (voChn < u32WndNum)
    {
        s32Ret = FY_MPI_VO_DisableChn(VoLayer, voChn);
        if (s32Ret != FY_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return FY_FAILURE;
        }
    }
    return s32Ret;
}

FY_S32 vo_get_chnRect(VO_CHN voChn, SAMPLE_VO_MODE_E enMode, SIZE_S *pstSize, RECT_S* pstRect)
{
    FY_S32 s32Ret = FY_SUCCESS;
    FY_U32 u32WndNum = 0;
    FY_U32 u32Square = 0;
    FY_U32 u32CellH = 0;
    FY_U32 u32CellW = 0;
    FY_S32 s32XO = 0, s32YO = 0;

    if (NULL == pstSize || NULL == pstRect) {
        s32Ret = FY_ERR_VO_NULL_PTR;
        goto EXIT;
    }

    switch (enMode) {
        case VO_MODE_1MUX:
            u32WndNum = 1;
            u32Square = 1;
            break;
        case VO_MODE_4MUX:
            u32WndNum = 4;
            u32Square = 2;
            break;
        case VO_MODE_9MUX:
            u32WndNum = 9;
            u32Square = 3;
            break;
        case VO_MODE_16MUX:
            u32WndNum = 16;
            u32Square = 4;
            break;
        case VO_MODE_1B_5S:
            u32WndNum = 6;
            u32Square = 3;
            break;
        case VO_MODE_1B_7S:
            u32WndNum = 8;
            u32Square = 4;
            break;
        case VO_MODE_1L_1R:
            u32WndNum = 2;
            u32Square = 2;
            break;
        case VO_MODE_25MUX:
            u32WndNum = 25;
            u32Square = 5;
            break;
        case VO_MODE_36MUX:
            u32WndNum = 36;
            u32Square = 6;
            break;
        case VO_MODE_49MUX:
            u32WndNum = 49;
            u32Square = 7;
            break;
        case VO_MODE_64MUX:
            u32WndNum = 64;
            u32Square = 8;
            break;
        default:
            break;
    }

    if (VO_MODE_1L_1R == enMode) {
        u32CellW = ALIGN_BACK(pstSize->u32Width/u32Square,  16);
        u32CellH = ALIGN_BACK(pstSize->u32Height/2, 2);
        s32XO = ALIGN_BACK((pstSize->u32Width - u32CellW*u32Square)/2, 2);
        s32YO = ALIGN_BACK((pstSize->u32Height - u32CellH)/2, 2);
    } else {
        u32CellW = ALIGN_BACK(pstSize->u32Width/u32Square,  16);
        u32CellH = ALIGN_BACK(pstSize->u32Height/u32Square, 2);
        s32XO = ALIGN_BACK((pstSize->u32Width - u32CellW*u32Square)/2, 2);
        s32YO = ALIGN_BACK((pstSize->u32Height - u32CellH*u32Square)/2, 2);
    }


    if (voChn < u32WndNum) {
        if (VO_MODE_1L_1R == enMode) {
            if (0 == voChn) {
                pstRect->s32X = s32XO;
                pstRect->s32Y = s32YO;
                pstRect->u32Width = u32CellW;
                pstRect->u32Height = u32CellH;
            } else if (1 == voChn){
                pstRect->s32X = s32XO + u32CellW;
                pstRect->s32Y = s32YO;
                pstRect->u32Width = u32CellW;
                pstRect->u32Height = u32CellH;
            }
        } else {
            if (VO_MODE_1B_5S == enMode || VO_MODE_1B_7S == enMode) {
                if (0 == voChn) {
                    pstRect->s32X = s32XO;
                    pstRect->s32Y = s32YO;
                } else if (voChn >= u32WndNum - u32Square) {
                    pstRect->s32X = u32CellW * (voChn + u32Square - u32WndNum) + s32XO;
                    pstRect->s32Y = u32CellH * (u32Square - 1) + s32YO;
                } else {
                    pstRect->s32X = u32CellW * (u32Square - 1) + s32XO;
                    pstRect->s32Y = u32CellH * (voChn - 1) + s32YO;
                }
            } else {
                pstRect->s32X = u32CellW * (voChn % u32Square) + s32XO;
                pstRect->s32Y = u32CellH * (voChn / u32Square) + s32YO;
            }

            if ((0 == voChn) && (VO_MODE_1B_5S == enMode || VO_MODE_1B_7S == enMode)) {
                pstRect->u32Width = u32CellW * (u32Square -1);
                pstRect->u32Height = u32CellH * (u32Square -1);
            } else {
                pstRect->u32Width = u32CellW;
                pstRect->u32Height = u32CellH;
            }
        }
    }


EXIT:

    return s32Ret;
}
