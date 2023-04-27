#ifndef _FHW_VGS_H
#define _FHW_VGS_H

#include "fy_common.h"
#include "fy_comm_vo.h"

#include "mpi_vgs.h"




FY_S32 vdec_vgs_init_layer(FY_S32 s32ChnNum, int enMode,VO_LAYER VoLayer);
FY_S32 vdec_vgs_deinit(FY_S32 s32ChnNum);
FY_S32 vgs_bind_vo(VGS_CHN VgsChn,VO_LAYER VoLayer, VO_CHN VoChn);
FY_S32 vgs_unbind_vo(VGS_CHN VgsChn,VO_LAYER VoLayer, VO_CHN VoChn);
#endif