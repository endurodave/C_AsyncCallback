#include "callback.h"
#include "WorkerThreadStd.h"
#include "SysData.h"
#include "SysDataNoLock.h"
#include "fb_allocator.h"
#include <iostream>
#include <string.h>

// main.cpp
// @see https://github.com/endurodave/C_AsyncCallback
// David Lafreniere, Aug 2020.

using namespace std;

struct TestStruct
{
    INT id;
};

// Maximum allowed registered callbacks
#define MAX_REGISTER  3

// Create a TestCb asynchronous callback that takes an integer pointer argument
CB_DECLARE(TestCb, int*)
CB_DEFINE(TestCb, int*, sizeof(int), MAX_REGISTER)

// Create a TestStrCb asynchronous callback that takes a char pointer argument
CB_DECLARE(TestStrCb, const char*)
CB_DEFINE(TestStrCb, const char*, sizeof(char), MAX_REGISTER)

void TestCallback1(int* val, void* userData)
{
    cout << "TestCallback1: " << *val << endl;
}

void TestCallback2(int* val, void* userData)
{
    // Typecast userData back to a TestStruct*
    TestStruct* pTestStruct = (TestStruct*)userData;

    cout << "TestCallback2: " << *val << ", id= " << pTestStruct->id << endl;
}

void TestStrCallback(const char* str, void* userData)
{
    cout << "TestStrCallback: " << str << endl;
}

void SysDataCallback(const SystemModeData* data, void* userData)
{
    cout << "SysDataCallback: " << data->CurrentSystemMode << endl;
}

void SysDataNoLockCallback(const SystemModeData* data, void* userData)
{
    cout << "SysDataNoLockCallback: " << data->CurrentSystemMode << endl;
}

int main()
{
    BOOL success;
	int data = 123;
    const char* strData = "Hello World!";
    TestStruct testStruct;
    testStruct.id = 555;

    // Initialize modules
    ALLOC_Init();
	CB_Init();
    SD_Init();
    SDNL_Init();
    CreateThreads();

    // Register to receive a synchronous callback
    success = CB_Register(TestCb, TestCallback1, NULL, NULL);

    // Register to receive asychronous callbacks on thread 1 and 2
	success = CB_Register(TestCb, TestCallback1, DispatchCallbackThread1, NULL);
    success = CB_Register(TestCb, TestCallback2, DispatchCallbackThread2, &testStruct);

    // Invoke the callbacks
	CB_Invoke(TestCb, &data);

    // CB_InvokeArray character array example
    CB_Register(TestStrCb, TestStrCallback, DispatchCallbackThread1, NULL);
    CB_InvokeArray(TestStrCb, strData, strlen(strData), sizeof(char));

    // Register to receive asynchronous callbacks from SysData
    CB_Register(SystemModeChangedCb, SysDataCallback, DispatchCallbackThread1, NULL);

    // Call CB_IsRegistered to check if a callback is registered
    if (CB_IsRegistered(SystemModeChangedCb, SysDataCallback, DispatchCallbackThread1))
    {
        // Set SysData system mode
        SD_SetSystemMode(STARTING);
        SD_SetSystemMode(NORMAL);
    }

    // SysDataNoLock example
    CB_Register(SystemModeChangedNoLockCb, SysDataNoLockCallback, DispatchCallbackThread2, NULL);
    SDNL_SetSystemMode(STARTING);
    SDNL_SetSystemMode(NORMAL);

    // Give time for message processing on worker threads
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Unregister from all callbacks
    CB_Unregister(TestCb, TestCallback1, NULL);
    CB_Unregister(TestCb, TestCallback1, DispatchCallbackThread1);
    CB_Unregister(TestCb, TestCallback2, DispatchCallbackThread2);
    CB_Unregister(SystemModeChangedCb, SysDataCallback, DispatchCallbackThread1);
    CB_Unregister(SystemModeChangedNoLockCb, SysDataNoLockCallback, DispatchCallbackThread2);
    CB_Unregister(TestStrCb, TestStrCallback, DispatchCallbackThread1);

    // Cleanup before exit
    SDNL_Term();
    SD_Term();
    CB_Term();
    ALLOC_Term();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}

