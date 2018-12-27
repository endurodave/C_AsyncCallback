#include "SysDataNoLock.h"
#include "LockGuard.h"

// Define a public SysData callback interface
CB_DEFINE(SystemModeChangedNoLockCb, const SystemModeData*, sizeof(SystemModeData), 2)

// Define a private callback interface
CB_DECLARE(SetSystemModeCb, SystemModeType*)
CB_DEFINE(SetSystemModeCb, SystemModeType*, sizeof(SystemModeType), 1)

// The current system mode data
static SystemModeType _systemMode;

extern BOOL DispatchCallbackThread1(const CB_CallbackMsg* cbMsg);
static void SDNL_SetSystemModePrivate(SystemModeType* systemMode, void* userData);

//----------------------------------------------------------------------------
// SDNL_Init
//----------------------------------------------------------------------------
void SDNL_Init(void)
{
    // Register with private callback
    CB_Register(SetSystemModeCb, SDNL_SetSystemModePrivate, DispatchCallbackThread1, NULL);
}

//----------------------------------------------------------------------------
// SDNL_Term
//----------------------------------------------------------------------------
void SDNL_Term(void)
{
    CB_Unregister(SetSystemModeCb, SDNL_SetSystemModePrivate, DispatchCallbackThread1);
}

//----------------------------------------------------------------------------
// SDNL_SetSystemMode
//----------------------------------------------------------------------------
void SDNL_SetSystemMode(SystemModeType systemMode)
{
    // Invoke the private callback. SDNL_SetSystemModePrivate() will be called
    // on DispatchCallbackThread1.
    CB_Invoke(SetSystemModeCb, &systemMode);
}

//----------------------------------------------------------------------------
// SDNL_SetSystemModePrivate
//----------------------------------------------------------------------------
static void SDNL_SetSystemModePrivate(SystemModeType* systemMode, void* userData)
{
    // Create the callback data
    SystemModeData callbackData;
    callbackData.PreviousSystemMode = _systemMode;
    callbackData.CurrentSystemMode = *systemMode;

    // Update the system mode
    _systemMode = *systemMode;

    // Callback all registered subscribers
    CB_Invoke(SystemModeChangedNoLockCb, &callbackData);
}