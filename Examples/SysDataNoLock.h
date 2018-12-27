#ifndef _SYS_DATA_NO_LOCK_H
#define _SYS_DATA_NO_LOCK_H

#include "callback.h"
#include "SysData.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declare a public SysData callback interface
CB_DECLARE(SystemModeChangedNoLockCb, const SystemModeData*)

void SDNL_Init(void);
void SDNL_Term(void);
void SDNL_SetSystemMode(SystemModeType systemMode);

#ifdef __cplusplus
}
#endif

#endif