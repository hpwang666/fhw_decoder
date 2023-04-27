#ifndef _FHW_SYS_H
#define _FHW_SYS_H

#include "fy_common.h"
#include "fy_comm_sys.h"
#include "fy_comm_vb.h"
#include "mpi_sys.h"
#include "mpi_vb.h"


#define SAMPLE_SYS_ALIGN_WIDTH  16

FY_S32 sys_init(VB_CONF_S *pstVbConf);
FY_VOID sys_exit(void);

#endif
