#ifndef _FHW_VDEC_H
#define _FHW_VDEC_H

#include "fy_common.h"
#include "fy_comm_sys.h"
#include "fy_comm_vb.h"
#include "common.h"


int vdec_start_mux_voChn_ext(FY_U32	s32ChnNum, int enMode,  int video_width, int video_height, PAYLOAD_TYPE_E pt_types[],int fb_cnt, int bind,VO_LAYER VoLayer,VO_CHN startVoChn);

FY_S32 vdec_load_userPic(char* fname, FY_U32 w,FY_U32 h, PIXEL_FORMAT_E format,VIDEO_FRAME_INFO_S* pUserFrame);

FY_VOID	vdec_mod_commPoolConf_ext(VB_CONF_S *pstModVbConf, PAYLOAD_TYPE_E *enTypes, SIZE_S *pstSize, FY_S32 s32ChnNum,FY_S32  fbCnt);
FY_S32	vdec_init_modCommVb(VB_CONF_S *pstModVbConf);
FY_S32 vdec_vo_init_layer(int enMode, VO_LAYER VoLayer, VO_CHN startVoChn, FY_S32 s32Chn);
FY_S32 vdec_vgs_bind_vo_layer(FY_S32 s32ChnNum, VO_LAYER VoLayer,VO_CHN startVoChn);
FY_S32 vdec_vgs_unbind_vo_layer(FY_S32 s32ChnNum, VO_LAYER VoLayer,VO_CHN startVoChn);
FY_VOID	vdec_chnAttr(FY_S32 s32ChnNum,	VDEC_CHN_ATTR_S	*pstVdecChnAttr, PAYLOAD_TYPE_E	enTypes[],	SIZE_S *pstSize);
FY_S32 vdec_bind_vgs(VDEC_CHN VdChn, VGS_CHN VgsChn);
FY_S32 vdec_unbind_vgs(VDEC_CHN VdChn, VGS_CHN VgsChn);
FY_S32 vdec_vo_deinit_layer(int enMode, VO_LAYER VoLayer,VO_CHN startVoChn, FY_S32 s32Chn);
FY_S32 vdec_start(FY_S32 s32ChnNum, VDEC_CHN_ATTR_S *pstAttr, FY_U32 u32BlkCnt);
FY_S32 vdec_stop(FY_S32 s32ChnNum);
#endif
