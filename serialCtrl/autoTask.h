
#ifndef _AUTOTASKS_H
#define _ALLTASKS_H

#include "osal.h"

#define OSAL_PRINTF (flag_base) 
#define OSAL_VO (flag_base<<1) 




int osal_auto_vo(int task_id,int events);



#endif
