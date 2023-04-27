#ifndef _FHW_VO_H
#define _FHW_VO_H

#include "fy_common.h"
#include "fy_comm_sys.h"
#include "fy_comm_vo.h"
#include "common.h"

typedef enum sample_vo_mode_e
{
    VO_MODE_1MUX    = 0,
    VO_MODE_4MUX    = 1,
    VO_MODE_9MUX    = 2,
    VO_MODE_16MUX   = 3,
    VO_MODE_1B_5S   = 4,
    VO_MODE_1B_7S   = 5,
    VO_MODE_1L_1R   = 6,
    VO_MODE_25MUX   = 7,
    VO_MODE_36MUX   = 8,
    VO_MODE_4T_4B   = 9,
    VO_MODE_49MUX   = 10,
    VO_MODE_64MUX   = 11,
    VO_MODE_BUTT
}SAMPLE_VO_MODE_E;


#define FY_VO_DEV_DHD0          0
#define FY_VO_LAYER_VHD0        0
#define FY_VO_DEV_DSD0          1
#define FY_VO_LAYER_VSD0        1
#define FY_VO_LAYER_VPIP        2

FY_S32 vo_init(FY_BOOL bIsHD, VO_INTF_SYNC_E enIntf);
FY_S32 vo_deinit(FY_BOOL bIsHD);
FY_S32 vo_getWH(VO_INTF_SYNC_E enIntfSync, FY_U32 *pu32W,FY_U32 *pu32H, FY_U32 *pu32Frm);
FY_S32 vo_start_singleLayer(VO_LAYER VoLayer,const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr, FY_BOOL isSingle);
FY_S32 vo_get_chnRect(VO_CHN voChn, SAMPLE_VO_MODE_E enMode, SIZE_S *pstSize, RECT_S* pstRect);
FY_S32 vo_start_chnOne(VO_LAYER VoLayer, VO_CHN voChn, SAMPLE_VO_MODE_E enMode);
FY_S32 vo_stop_chnOne(VO_LAYER VoLayer, VO_CHN voChn, SAMPLE_VO_MODE_E enMode);

#endif