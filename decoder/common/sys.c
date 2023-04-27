
#include <stdlib.h>
#include <stdio.h>
#include "sys.h"


FY_S32 sys_init(VB_CONF_S *pstVbConf)
{
    MPP_SYS_CONF_S stSysConf = {0};
    FY_S32 s32Ret = FY_FAILURE;
    FY_S32 i;

    FY_MPI_SYS_Exit();

    for(i=0;i<VB_MAX_USER;i++)
    {
         FY_MPI_VB_ExitModCommPool(i);
    }
    for(i=0; i<VB_MAX_POOLS; i++)
    {
         FY_MPI_VB_DestroyPool(i);
    }
    FY_MPI_VB_Exit();

    if (NULL == pstVbConf)
    {
        printf("input parameter is null, it is invaild!\n");
        return FY_FAILURE;
    }

    s32Ret = FY_MPI_VB_SetConf(pstVbConf);
    if (FY_SUCCESS != s32Ret)
    {
        printf("FY_MPI_VB_SetConf failed!\n");
        return FY_FAILURE;
    }

    s32Ret = FY_MPI_VB_Init();
    if (FY_SUCCESS != s32Ret)
    {
        printf("FY_MPI_VB_Init failed!\n");
        return FY_FAILURE;
    }

    stSysConf.u32AlignWidth = SAMPLE_SYS_ALIGN_WIDTH;
    s32Ret = FY_MPI_SYS_SetConf(&stSysConf);
    if (FY_SUCCESS != s32Ret)
    {
        printf("FY_MPI_SYS_SetConf failed\n");
        return FY_FAILURE;
    }

    s32Ret = FY_MPI_SYS_Init();
    if (FY_SUCCESS != s32Ret)
    {
        printf("FY_MPI_SYS_Init failed!\n");
        return FY_FAILURE;
    }

    return FY_SUCCESS;
}



// FY_VOID fy_vda_MdStopAll()
// {
// 	FY_U32 i;

// 	for(i=0;i<MAX_MD_CHANNEL_NUM;i++)
// 	{
// 		if(g_MotionDetection.uMD[i].bUse)
// 		{
// 			MPI_VDA_MD_DeInit(i);
// 			FY_MPI_VPSS_SetChnMode(g_MotionDetection.uMD[i].s32Grp,VPSS_CHN0,&g_MotionDetection.uMD[i].stOrigVpssMode);
// 		}
// 	}
// 	memset(g_MotionDetection.uMD,0,sizeof(MD_CD_Param)*MAX_MD_CHANNEL_NUM);
// }


FY_VOID sys_exit(void)
{

    FY_S32 i;

    FY_MPI_SYS_Exit();
    for(i=0;i<VB_MAX_USER;i++)
    {
         FY_MPI_VB_ExitModCommPool(i);
    }
    for(i=0; i<VB_MAX_POOLS; i++)
    {
         FY_MPI_VB_DestroyPool(i);
    }
    FY_MPI_VB_Exit();
    return;
}