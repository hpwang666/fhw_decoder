

 /******************************************************************************

	Date        : 2020/5/1
    Author      : hpwang 
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "vo.h"
#include "decoder.h"
#include "sys.h"

#undef  _DEBUG
#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif 


//设计支持16通道的解码

FY_VOID vdec_handle_sig(FY_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        //dec_sys_exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}



FY_S32 decode_h264(int accessable)
{
	int i=0;
    FY_U32 u32BlkSize = 0;
	FY_S32 s32Ret=0;
    VB_CONF_S stVbConf ;

    PAYLOAD_TYPE_E pt_types[VDEC_MAX_CHN_NUM];
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 1;

    u32BlkSize = 1920 * 1440 * 3/2;
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 4;

    sys_init(&stVbConf);
    vo_init(FY_TRUE, VO_OUTPUT_1080P60);

    for(i=0;i<VDEC_MAX_CHN_NUM;i++){
        pt_types[i]=PT_H264;
    }
    vdec_start_mux_voChn_ext(VDEC_CHN_NUM_4, VO_MODE_4MUX, HD_WIDTH,HD_HEIGHT,\
		   					pt_types,3, 1,FY_VO_LAYER_VHD0,0);
    return s32Ret;
}
