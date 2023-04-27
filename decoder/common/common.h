#ifndef __SAMPLE_COMM_H__
#define __SAMPLE_COMM_H__

#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "fy_common.h"
#include "fy_comm_sys.h"
#include "fy_comm_vb.h"
#include "fy_comm_vi.h"
#include "fy_comm_vo.h"
#include "fy_comm_venc.h"
#include "fy_comm_vpss.h"
#include "fy_comm_vdec.h"
#include "fy_comm_region.h"
#include "fy_comm_adec.h"
#include "fy_comm_aenc.h"
#include "fy_comm_aio.h"
#include "fy_comm_vgs.h"
#include "fy_defines.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_vgs.h"
#include "mpi_region.h"
#include "mpi_adec.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_ao.h"




#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */

/*******************************************************
    macro define
*******************************************************/
#define CHECK_CHN_RET(express,Chn,name)\
	do{\
		FY_S32 Ret;\
		Ret = express;\
		if (FY_SUCCESS != Ret)\
		{\
			printf("\033[0;31m%s chn %d failed at %s: LINE: %d with %#x!\033[0;39m\n", name, Chn, __FUNCTION__, __LINE__, Ret);\
			fflush(stdout);\
			return Ret;\
		}\
	}while(0)

#define CHECK_RET(express,name)\
    do{\
        FY_S32 Ret;\
        Ret = express;\
        if (FY_SUCCESS != Ret)\
        {\
            printf("\033[0;31m%s failed at %s: LINE: %d with %#x!\033[0;39m\n", name, __FUNCTION__, __LINE__, Ret);\
            return Ret;\
        }\
    }while(0)

//#define SAMPLE_GLOBAL_NORM	    VIDEO_ENCODING_MODE_PAL
#define SAMPLE_PIXEL_FORMAT         PIXEL_FORMAT_YUV_SEMIPLANAR_422

#define NVP6134_FILE    "/dev/nc_vdec"


#define SAMPLE_AUDIO_PTNUMPERFRM	320
#define SAMPLE_AUDIO_TLV320_DEV		1
#define SAMPLE_AUDIO_TW2865_DEV		0
#define SAMPLE_AUDIO_HDMI_AO_DEV	I2S_DEV3
#if(defined(FY01) || defined(FY10) || defined(MC6630) || defined(MC6830)|| defined(MC6650))
#define SAMPLE_AUDIO_AI_DEV		I2S_DEV0
#define SAMPLE_AUDIO_AO_DEV		I2S_DEV1
#elif defined(MC6850)
#define SAMPLE_AUDIO_AI_DEV		I2S_DEV1
#define SAMPLE_AUDIO_AO_DEV		I2S_DEV1
#else
#define SAMPLE_AUDIO_AI_DEV		DEV_ACW_ID_0
#define SAMPLE_AUDIO_AO_DEV		DEV_ACW_ID_0
#endif
#define SAMPLE_AUDIO_RATE		AUDIO_SAMPLE_RATE_8000
#define SAMPLE_AUDIO_FRM_SIZE		(SAMPLE_AUDIO_RATE*2/25)
#define SAMPLE_AAC_FRM_SIZE		1024*2
#define FRMAE_SIZE(fs,bit,num)		(fs/num)*(bit/8)
#define CNT_PER_S			25

#if (defined(MC6630) || defined(MC6650))
#define AI_CHN_NUM			AUDIO_CHN_NUM_16
#else
#define AI_CHN_NUM			AUDIO_CHN_NUM_2
#endif

#if defined(MC6850)
#define CODEC_MASTER_MODE		FY_FALSE
#else
#define CODEC_MASTER_MODE		FY_FALSE
#endif

#define SAMPLE_CIF_H264_PATH "../common/CIF.h264"
#define SAMPLE_1080P_H264_PATH "../common/1080P.h264"
#define SAMPLE_1080P_H265_PATH "../common/1080P.h265"
#define SAMPLE_4K_H264_PATH "../common/tmp1"
#define SAMPLE_4K_H265_PATH "../common/tmp2"
#define SAMPLE_1080P_MPEG4_PATH "../common/1080P.mpeg4"
#define SAMPLE_FIELD_H264_PATH "../common/D1_field.h264"
#define SAMPLE_1080P_JPEG_PATH "../common/1080P.jpg"
#define SAMPLE_4K_JPEG_PATH "../common/tmp3"

#define SAMPLE_MAX_VDEC_CHN_CNT 8


#define ALIGN_UP(x, a)              ((x+a-1)&(~(a-1)))
#define ALIGN_BACK(x, a)              ((a) * (((x) / (a))))

#define VO_BKGRD_BLUE           0x0000FF

#define HD_WIDTH                1920
#define HD_HEIGHT               1088

#define D1_WIDTH                720
#define D1_HEIGHT               576
#define D1_WIDTH_704            704

#define _720P_WIDTH              1280
#define _720P_HEIGHT             720

#define GOSD_DEFAULT_PIXEDEPTH 16

#define SAMPLE_PRT(fmt...)   \
    do {\
        printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
        printf(fmt);\
       }while(0)

typedef struct sample_vi_param_s
{
    FY_S32 s32ViDevCnt;        // VI Dev Total Count
    FY_S32 s32ViDevInterval;   // Vi Dev Interval
    FY_S32 s32ViChnCnt;        // Vi Chn Total Count
    FY_S32 s32ViChnInterval;   // VI Chn Interval
}SAMPLE_VI_PARAM_S;
#define VPSS_GRP_NUM_TEST 4



typedef struct fysample_MEMBUF_S
{
    VB_BLK  hBlock;
    VB_POOL hPool;
    FY_U32  u32PoolId;

    FY_U32  u32PhyAddr;
    FY_U8   *pVirAddr;
    FY_S32  s32Mdev;
} SAMPLE_MEMBUF_S;

typedef enum sample_rgn_change_type_e
{
    RGN_CHANGE_TYPE_FGALPHA = 0,
    RGN_CHANGE_TYPE_BGALPHA,
    RGN_CHANGE_TYPE_LAYER
}SAMPLE_RGN_CHANGE_TYPE_EN;

typedef enum
{
    HIFB_LAYER_0 = 0x0,
    HIFB_LAYER_1,
    HIFB_LAYER_2,
    HIFB_LAYER_CURSOR_0,
    HIFB_LAYER_ID_BUTT
} HIFB_LAYER_ID_E;

typedef enum PLAYBACK_CMD_CODE_E
{
	PLAYBACK_CMD_CODE_SPEED = 0, /* cmd_parm: frame rate,*/
	PLAYBACK_CMD_CODE_PAUSE_RESUME,  /* cmd_parm: 0-pause, 1-resume */
	PLAYBACK_CMD_CODE_USERPIC,  /* cmd_parm: 0-hide, 1-show */
	PLAYBACK_CMD_CODE_AJUST_CHANNEL, /* cmd_parm: -1: decrase, 1: increase */
	PLAYBACK_CMD_CODE_AUTO_CHANNELS, /* cmd_parm: 0: stop, 1: start */
	PLAYBACK_CMD_CODE_AUTO_CHANNELS_ONOFF, /* cmd_parm: 0: stop, 1: start */
	PLAYBACK_CMD_CODE_ONOFF_CHANNEL,
	PLAYBACK_CMD_CODE_AUTO_SPEED, /* cmd_parm: 0: stop, 1: start */
}PLAYBACK_CMD_CODE;

typedef enum fyAudioCodecType
{
    AUDIO_CODEC_INNER = 0,
    AUDIO_CODEC_TLV320,
    AUDIO_CODEC_HDMI,
    AUDIO_CODEC_TW2865,
    AUDIO_CODEC_TP28XX,
    AUDIO_CODEC_ACWRAPPER,
    AUDIO_CODEC_NAU88C10,
    AUDIO_CODEC_ALC5616,
    AUDIO_CODEC_BUTT
}AudioCodecType;

typedef enum __AudioFileT
{
	FILE_TYPE_AI,
	FILE_TYPE_AENC,
	FILE_TYPE_TOTAL
}AudioFileT;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __SAMPLE_COMMON_H__ */
