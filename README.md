# Asynchronous Multicast Callbacks in C
Simplify passing data between threads with this portable C language callback library.

Originally published on CodeProject at: <a href="https://www.codeproject.com/Articles/1272894/Asynchronous-Multicast-Callbacks-in-C"><strong>Asynchronous Multicast Callbacks in C</strong></a>

<h2>Introduction</h2>

<p>Callbacks are a powerful concept used to reduce the coupling between two pieces of code. On a multithreaded system, callbacks have limitations. What I&#39;ve always wanted was a callback mechanism that crosses threads and handles all the low-level machinery to get my event data from one thread to another safely. A portable and easy to use framework. No more monster switch statements inside a thread loop that typecast OS message queue <code>void*</code> values based upon an enumeration. Create a callback. Register a callback. And the framework automagically invokes the callback with data arguments on a user specified target thread is the goal.&nbsp;</p>

<p>On systems that use event loops, a message queue and switch statement are sometimes employed to hande&nbsp;incoming messages. Over time, the event loop function grows larger and larger as more message types are dispatched to the thread. Weird hacks are added in order to solve various system issues. Ultimately what ensues is a fragile, hard to read function that is constantly touched by engineers over the life of the project. This solution attempts to simplify and standardize the event loop in order to generalize the movement of data between threads.&nbsp;</p>

<p>I&rsquo;ve documented C++ solutions to asynchronous multicast callbacks on two previous CodeProject articles. One version supports a single callback argument. The other employs a multi-argument C++ delegate paradigm&nbsp;(see the <strong>References </strong>section below for C++ articles).&nbsp;</p>

<p>This third time around the solution is implemented in C. The callback solution presented here provides the following features:</p>

<ul>
	<li><strong>Asynchronous callbacks</strong> &ndash; support asynchronous callbacks to and from any thread</li>
	<li><strong>Thread targeting</strong> &ndash; specify the destination thread for the asynchronous callback</li>
	<li><strong>Callbacks </strong>&ndash; invoke any C or C++ free/static function with a matching signature</li>
	<li><strong>Type safe</strong> &ndash; user defined, type safe callback function data argument</li>
	<li><strong>Multicast callbacks</strong> &ndash; store multiple callbacks within an array for sequential invocation</li>
	<li><strong>Thread-safe</strong> &ndash; suitable for use on a multi-threaded system</li>
	<li><strong>Compact </strong>&ndash; small, easy to maintain code base consuming minimal code space</li>
	<li><strong>Portable </strong>&ndash; portable to an embedded or PC-based platform</li>
	<li><strong>Any compiler</strong> &ndash; support for any C language compiler</li>
	<li><strong>Any OS</strong> - easy porting to any operating system</li>
	<li><strong>Elegant syntax</strong> &ndash; intuitive and easy to use</li>
</ul>

<p>The asynchronous callback paradigm significantly eases multithreaded application development by placing the callback function pointer and callback data onto the thread of control that you specify. Exposing a&nbsp;callback interface for a single module or an entire subsystem is extremely easy. The framework is no more difficult to use than a standard C callback but with more features.</p>

<p>This article proposes an inter-thread communication mechanism utilizing asynchronous multicast callbacks. The attached source code implements all features above, as I&#39;ll demonstrate.</p>

<h2>Callbacks Background</h2>

<p>The idea of a function callback is very useful. In callback terms, a publisher defines the callback signature and allows anonymous registration of a callback function pointer. A subscriber creates a function implementation conforming to the publisher&#39;s callback signature and registers a callback function pointer with the publisher at runtime. The publisher code knows nothing about the subscriber code &ndash; the registration and the callback invocation is anonymous.</p>

<p>Now, on a multithreaded system, you need understand synchronous vs. asynchronous callback invocations. If the callback is synchronous, the callback is executed on the caller&#39;s thread of control. If you put a break point inside the callback, the stack frame will show the publisher function call and the publisher callback all synchronously invoked. There are no multithreaded issues with this scenario as everything is running on a single thread.</p>

<p>If the publisher code has its own thread, it may invoke the callback function on its thread of control and not the subscriber&#39;s thread. A publisher invoked callback can occur at any time completely independent of the subscriber&rsquo;s thread of control. This cross-threading can cause problems for the subscriber if the callback code is not thread-safe since you now have another thread calling into subscriber code base at some unknown interval.</p>

<p>One solution for making a callback function thread-safe is to post a message to the subscriber&#39;s OS queue during the publisher&#39;s callback. The subscriber&#39;s thread later dequeues the message and calls an appropriate function. Since the callback implementation only posts a message, the callback, even if done asynchronously, is thread-safe. In this case, the asynchrony of a message queue provides the thread safety in lieu of software locks.</p>

<h2>Using the Code</h2>

<p>I&#39;ll first present how to use the code, and then get into the implementation details.</p>

<p>A publisher uses the <code>CB_DECLARE </code>macro to expose a callback interface to potential subscribers, typically within a header file. The first argument is the callback name. The second argument is the callback function argument type. In the example below, <code>int*</code> is the callback function argument.</p>

<pre lang="c++">
CB_DECLARE(TestCb, int*)</pre>

<p>The publisher uses the <code>CB_DEFINE </code>macro within a source file to complete the callback definition. The first argument is the callback name. The second argument is the callback function argument type. The third argument is the size of the data pointed to by the callback function argument. The last argument is the maximum number of subscribers that can register for callback notifications.&nbsp;</p>

<pre lang="c++">
CB_DEFINE(TestCb, int*, sizeof(int), MAX_REGISTER)</pre>

<p>To subscribe to callback, create a function (<code>static </code>class member or global) as shown. I&rsquo;ll explain why the function signature argument requires a <code>(int*, void*)</code> function signature shortly.</p>

<pre lang="c++">
void TestCallback1(int* val, void* userData)
{
    printf(&ldquo;TestCallback1 %d&rdquo;, *val);
}</pre>

<p>The subscriber registers to receive callbacks using the <code>CB_Register()</code> function macro. The first argument is the callback name. The second argument is a pointer to the callback function. The third argument is a pointer to a thread dispatch function or <code>NULL </code>if a synchronous callback is desired. And the last argument is a pointer to optional user data passed during callback invocation. The framework internally does nothing with user data other than pass it back to the callback function. The user data value can be anything the caller wants or <code>NULL</code>.</p>

<pre lang="c++">
CB_Register(TestCb, TestCallback1, DispatchCallbackThread1, NULL);</pre>

<p>On C/C++ mixed projects, the <code>userData </code>callback argument can be used to store a <code>this </code>class instance pointer. Pass a class <code>static </code>member function pointer for the callback function and a <code>this </code>pointer for user data to <code>CB_Register()</code>. Within the subscriber callback function, typecast <code>userData </code>back to a class instance pointer. This provides an easy means of accessing class instance functions and data within a <code>static </code>callback function. &nbsp;</p>

<p>Use <code>CB_Invoke()</code> when a publisher needs to invoke the callback for all registered subscribers. The function dispatches the callback and data argument onto the destination thread of control. In the example below, <code>TestCallback1() </code>is called on <code>DispatchCallbackThread1</code>.&nbsp;</p>

<pre lang="c++">
int data = 123;
CB_Invoke(TestCb, &amp;data);</pre>

<p>Use <code>CB_Unregister()</code> to unsubscribe from a callback.</p>

<pre lang="c++">
CB_Unregister(TestCb, TestCallback1, DispatchCallbackThread1);</pre>

<p>Asynchronous callbacks are easily used to add asynchrony to both incoming and outgoing API interfaces. The following examples show how.</p>

<h2>SysData Publisher Example</h2>

<p><code>SysData </code>is a simple module showing how to expose an <em>outgoing </em>asynchronous interface. The module stores system data and provides asynchronous subscriber notifications when the mode changes. The module interface is shown below.</p>

<pre lang="c++">
typedef enum
{
    STARTING,
    NORMAL,
    SERVICE,
    SYS_INOP
} SystemModeType;

typedef struct
{
    SystemModeType PreviousSystemMode;
    SystemModeType CurrentSystemMode;
} SystemModeData;

// Declare a SysData callback interface
CB_DECLARE(SystemModeChangedCb, const SystemModeData*)

void SD_Init(void);
void SD_Term(void);
void SD_SetSystemMode(SystemModeType systemMode);
</pre>

<p>The publisher callback interface is <code>SystemModeChangedCb</code>. Calling <code>SD_SetSystemMode()</code> saves the new mode into <code>_systemMode </code>and notifies all registered subscribers.</p>

<pre lang="c++">
void SD_SetSystemMode(SystemModeType systemMode)
{
    LK_LOCK(_hLock);

    // Create the callback data
    SystemModeData callbackData;
    callbackData.PreviousSystemMode = _systemMode;
    callbackData.CurrentSystemMode = systemMode;

    // Update the system mode
    _systemMode = systemMode;

    // Callback all registered subscribers
    CB_Invoke(SystemModeChangedCb, &amp;callbackData);

    LK_UNLOCK(_hLock);
}
</pre>

<h2>SysData Subscriber Example</h2>

<p>The subscriber creates a&nbsp;callback function that conforms to the publisher&#39;s callback function signature.&nbsp;</p>

<pre lang="c++">
void SysDataCallback(const SystemModeData* data, void* userData)
{
    cout &lt;&lt; &quot;SysDataCallback: &quot; &lt;&lt; data-&gt;CurrentSystemMode &lt;&lt; endl;
}
</pre>

<p>At runtime, <code>CB_Register()</code> is used to register for <code>SysData </code>callbacks on <code>DispatchCallbackThread1</code>.&nbsp;</p>

<pre lang="c++">
CB_Register(SystemModeChangedCb, SysDataCallback, DispatchCallbackThread1, NULL);</pre>

<p>When <code>SD_SetSystemMode()</code> is called, any client interested in the mode changes are notified asynchronously on their desired execution thread.</p>

<pre lang="c++">
SD_SetSystemMode(STARTING);
SD_SetSystemMode(NORMAL);
</pre>

<h2>SysDataNoLock Publisher Example</h2>

<p><code>SysDataNoLock </code>is an alternate implementation that uses a private callback for setting the system mode asynchronously and without locks.</p>

<pre lang="c++">
// Declare a public SysData callback interface
CB_DECLARE(SystemModeChangedNoLockCb, const SystemModeData*)

void SDNL_Init(void);
void SDNL_Term(void);
void SDNL_SetSystemMode(SystemModeType systemMode);
</pre>

<p>The initialize function registers with the private <code>SetSystemModeCb</code> callback.&nbsp;</p>

<pre lang="c++">
// Define a private callback interface
CB_DECLARE(SetSystemModeCb, SystemModeType*)
CB_DEFINE(SetSystemModeCb, SystemModeType*, sizeof(SystemModeType), 1)

void SDNL_Init(void)
{
    // Register with private callback
    CB_Register(SetSystemModeCb, SDNL_SetSystemModePrivate, DispatchCallbackThread1, NULL);
}
</pre>

<p>The <code>SSNL_SetSystemMode()</code> function below is an example of an asynchronous <em>incoming </em>interface. To the caller, it looks like a normal function, but, under the hood,&nbsp;a private function call is invoked asynchronously. In this case, invoking <code>SetSystemModeCb </code>causes <code>SDNL_SetSystemModePrivate()</code> to be called on <code>DispatchCallbackThread1</code>.</p>

<pre lang="c++">
void SDNL_SetSystemMode(SystemModeType systemMode)
{
    // Invoke the private callback. SDNL_SetSystemModePrivate() will be called
    // on DispatchCallbackThread1.
    CB_Invoke(SetSystemModeCb, &amp;systemMode);
}
</pre>

<p>Since this private function is always invoked asynchronously on <code>DispatchCallbackThread1 </code>it doesn&#39;t require locks.</p>

<pre lang="c++">
static void SDNL_SetSystemModePrivate(SystemModeType* systemMode, void* userData)
{
    // Create the callback data
    SystemModeData callbackData;
    callbackData.PreviousSystemMode = _systemMode;
    callbackData.CurrentSystemMode = *systemMode;

    // Update the system mode
    _systemMode = *systemMode;

    // Callback all registered subscribers
    CB_Invoke(SystemModeChangedNoLockCb, &amp;callbackData);
}
</pre>

<h2>Callback Signature Limitations</h2>

<p>This design has the following limitations imposed on all callback functions:</p>

<ol>
	<li>Each callback handles a single user-defined argument type.</li>
	<li>The argument may be a <code>const </code>or non-<code>const</code> pointer (e.g. <code>const MyData*</code> or <code>MyData*</code>).</li>
	<li>The two callback function arguments are always: <code>MyData*</code> and <code>void*</code>.</li>
	<li>Each callback has a <code>void </code>return type.</li>
</ol>

<p>For instance, if a callback is declared as:</p>

<pre lang="c++">
CB_DECLARE(TestCb, const MyData*)</pre>

<p>The callback function signature is:</p>

<pre lang="c++">
void MyCallback(const MyData* data, void* userData);</pre>

<p>The design can be extended to support more than one argument if necessary. However, the design somewhat mimics what embedded programmers do all the time, which is something like:</p>

<ol>
	<li>Dynamically create an instance to a struct or class and populate data.</li>
	<li>Post a pointer to the data through an OS message as a <code>void*</code>.</li>
	<li>Get the data from the OS message queue and typecast the <code>void*</code> back to the original type.</li>
	<li>Delete the dynamically created data.</li>
</ol>

<p>In this design, the entire infrastructure happens automatically without any additional effort on the programmer&#39;s part. If multiple data parameters are required, they must be packaged into a single class/struct and used as the callback data argument.</p>

<h2>Implementation</h2>

<p>The number of lines of code for the callback framework is surprisingly low. Strip out the comments, and maybe a couple hundred lines of code that are (hopefully) easy to understand and maintain.</p>

<p>The implementation uses macros and token pasting to provide a unique type-safe interface for each callback. The token pasting operator (<code>##</code>) is used to merge two tokens when the preprocessor expands the macro. The <code>CB_DECLARE </code>macro is shown below.</p>

<pre lang="c++">
#define CB_DECLARE(cbName, cbArg) \
    typedef void(*cbName##CallbackFuncType)(cbArg cbData, void* cbUserData); \
    BOOL cbName##_Register(cbName##CallbackFuncType cbFunc, CB_DispatchCallbackFuncType cbDispatchFunc, void* cbUserData); \
    BOOL cbName##_IsRegistered(cbName##CallbackFuncType cbFunc, CB_DispatchCallbackFuncType cbDispatchFunc); \
    BOOL cbName##_Unregister(cbName##CallbackFuncType cbFunc, CB_DispatchCallbackFuncType cbDispatchFunc); \
    BOOL cbName##_Invoke(cbArg cbData); \
    BOOL cbName##_InvokeArray(cbArg cbData, size_t num, size_t size);
</pre>

<p>In the <code>SysData</code> example used above, the compiler preprocessor expands <code>CB_DECLARE </code>to:</p>

<pre lang="c++">
typedef void(*SystemModeChangedCbCallbackFuncType)(const SystemModeData* cbData, void* cbUserData); 

BOOL SystemModeChangedCb_Register(SystemModeChangedCbCallbackFuncType cbFunc, CB_DispatchCallbackFuncType cbDispatchFunc, void* cbUserData);
 
BOOL SystemModeChangedCb_IsRegistered(SystemModeChangedCbCallbackFuncType cbFunc, CB_DispatchCallbackFuncType cbDispatchFunc); 

BOOL SystemModeChangedCb_Unregister(SystemModeChangedCbCallbackFuncType cbFunc, CB_DispatchCallbackFuncType cbDispatchFunc); 

BOOL SystemModeChangedCb_Invoke(const SystemModeData* cbData); 

BOOL SystemModeChangedCb_InvokeArray(const SystemModeData* cbData, size_t num, size_t size);
</pre>

<p>Notice every <code>cbName##</code> location is replaced by the macro name argument, in this case, being <code>SystemModeChangedCb </code>from the declaration below.</p>

<pre lang="c++">
CB_DECLARE(SystemModeChangedCb, const SystemModeData*)</pre>

<p>Similarly, the <code>CB_DEFINE</code> macro expands to create the callback function implementations. Notice the macro provides a thin, type-safe wrapper around private functions such as <code>_CB_AddCallback()</code> and <code>_CB_Dispatch()</code>. If attempting to register the wrong function signature, the compiler generates an error or warning. The macros automate the monotonous, boilerplate code that you&rsquo;d normally write by hand.</p>

<p>The registered callbacks are stored in a <code>static </code>array of <code>CB_Info </code>instances. Calling&nbsp;<code>CB_Invoke(SystemModeChangedCb, &amp;callbackData)</code>&nbsp;executes <code>SystemModeChangedCb_Invoke()</code>. Then <code>_CB_Dispatch()</code>&nbsp;iterates over the <code>CB_Info </code>array and dispatches one&nbsp;<code>CB_CallbackMsg </code>message&nbsp;to each target thread. The message data is dynamically created to travel through an OS message queue.</p>

<pre lang="c++">
// Macro generated unique invoke function
BOOL SystemModeChangedCb_Invoke(const SystemModeData* cbData) 
{ 
    return _CB_Dispatch(&amp;SystemModeChangedCbMulticast[0], 2, cbData, sizeof(SystemModeData)); 
}

BOOL _CB_Dispatch(CB_Info* cbInfo, size_t cbInfoLen, const void* cbData, 
    size_t cbDataSize)
{
    BOOL invoked = FALSE;

    LK_LOCK(_hLock);

    // For each CB_Info instance within the array
    for (size_t idx = 0; idx&lt;cbInfoLen; idx++)
    {
        // Is a client registered?
        if (cbInfo[idx].cbFunc)
        {
            // Dispatch callback onto the OS task
            if (CB_DispatchCallback(&amp;cbInfo[idx], cbData, cbDataSize))
            {
                invoked = TRUE;
            }
        }
    }

    LK_UNLOCK(_hLock);
    return invoked;
}
</pre>

<p>The target OS task event loop dequeues a <code>CB_CallbackMsg*</code> and calls <code>CB_TargetInvoke()</code>. The dynamic data created is freed before the function exits.&nbsp;</p>

<pre lang="c++">
void CB_TargetInvoke(const CB_CallbackMsg* cbMsg)
{
    ASSERT_TRUE(cbMsg);
    ASSERT_TRUE(cbMsg-&gt;cbFunc);

    // Invoke callback function with the callback data
    cbMsg-&gt;cbFunc(cbMsg-&gt;cbData, cbMsg-&gt;cbUserData);

    // Free data sent through OS queue
    XFREE((void*)cbMsg-&gt;cbData);
    XFREE((void*)cbMsg);
} 
</pre>

<p>Asynchronous callbacks impose certain limitations because everything the callback destination thread must be created on the heap, packaged into a <code>CB_CallbackMsg </code>structure, and placed into an OS message queue.</p>

<p>The insertion into an OS queue is platform specific. The <code>CB_DispatchCallbackFuncType </code>function pointer <code>typedef</code> provides the OS queue interface to be implemented for each thread event loop on the target platform. See the <strong>Porting</strong> section below for a more complete discussion.</p>

<pre lang="c++">
typedef BOOL (*CB_DispatchCallbackFuncType)(const CB_CallbackMsg* cbMsg);</pre>

<p>Once the message is placed into the message queue, platform specific code unpacks the message and calls the <code>CB_TargetInvoke()</code> function and destroys dynamically allocated data. For this example, a simple <code>WorkerThreadStd </code>class provides the thread event loop leveraging&nbsp;the C++ thread support library. While this example uses C++ threads, the callback modules are written in plain C. Abstracting the OS details from the callback implementation makes this possible.&nbsp;</p>

<pre lang="c++">
void WorkerThread::Process()
{
    while (1)
    {
        ThreadMsg* msg = 0;
        {
            // Wait for a message to be added to the queue
            std::unique_lock&lt;std::mutex&gt; lk(m_mutex);
            while (m_queue.empty())
                m_cv.wait(lk);

            if (m_queue.empty())
                continue;

            msg = m_queue.front();
            m_queue.pop();
        }

        switch (msg-&gt;GetId())
        {
            case MSG_DISPATCH_DELEGATE:
            {
                ASSERT_TRUE(msg-&gt;GetData() != NULL);

                // Convert the ThreadMsg void* data back to a CB_CallbackMsg* 
                const CB_CallbackMsg* callbackMsg = static_cast&lt;const CB_CallbackMsg*&gt;(msg-&gt;GetData());

                // Invoke the callback on the target thread
                CB_TargetInvoke(callbackMsg);

                // Delete dynamic data passed through message queue
                delete msg;
                break;
            }
        }
    }
}
</pre>

<p>Notice the thread loop is unlike most systems that have a huge switch statement handling various incoming data messages, type casting <code>void*</code> data, then calling a specific function. The framework supports all callbacks with a single <code>WM_DISPATCH_DELEGATE</code> message. Once setup, the same small thread loop handles every callback. New publishers and subscribers come and go as the system is designed, but the code in-between doesn&#39;t change.</p>

<p>This is a huge benefit as on many systems getting data between threads takes a lot of manual steps. You constantly have to mess with each thread loop, create during sending, destroy data when receiving, and call various OS services and typecasts. Here you do none of that. All the stuff in-between is neatly handled for users.</p>

<h2>Heap</h2>

<p>The dynamic data is required to send data structures through the message queue. Remember, the data pointed to by your callback argument is bitwise copied during a callback.&nbsp;</p>

<p>On some systems, it is undesirable to use the heap. For those situations, I use a fixed block memory allocator. The <code>x_allocator</code> implementation solves the dynamic storage issues and is much faster than the global heap. To use, just define <code>USE_CALLBACK_ALLOCATOR </code>within <strong>callback.c</strong>. See the <strong>References </strong>section for more information on <code>x_allocator</code>.</p>

<h2>Porting</h2>

<p>The code is an easy port to any platform. There are only two OS services required: threads and a software lock. The code is separated into four directories.</p>

<ol>
	<li><strong>Callback </strong>- core library implementation files</li>
	<li><strong>Port </strong>&ndash; Windows-specific files (thread/lock)</li>
	<li><strong>Examples </strong>&ndash; sample code showing usage</li>
	<li><strong>VS2017 </strong>&ndash; Visual Studio 2017 project files</li>
</ol>

<p>Porting to another platform requires implementing a dispatch function that accepts a <code>const CB_CallbackMsg* </code>for each thread. The functions below show an example.</p>

<pre lang="c++">
// C language interface to a callback dispatch function
extern &quot;C&quot; BOOL DispatchCallbackThread1(const CB_CallbackMsg* cbMsg)
{
    workerThread1.DispatchCallback(cbMsg);
    return TRUE;
}

void WorkerThread::DispatchCallback(const CB_CallbackMsg* msg)
{
    ASSERT_TRUE(m_thread);

    // Create a new ThreadMsg
    ThreadMsg* threadMsg = new ThreadMsg(MSG_DISPATCH_DELEGATE, msg);

    // Add dispatch delegate msg to queue and notify worker thread
    std::unique_lock&lt;std::mutex&gt; lk(m_mutex);
    m_queue.push(threadMsg);
    m_cv.notify_one();
}
</pre>

<p>The thread event loop gets the message and calls the <code>CB_TargetInvoke()</code> function. The data sent through the queue is deleted once complete.&nbsp;</p>

<pre lang="c++">
case MSG_DISPATCH_DELEGATE:
{
    ASSERT_TRUE(msg-&gt;GetData() != NULL);

    // Convert the ThreadMsg void* data back to a CB_CallbackMsg* 
    const CB_CallbackMsg* callbackMsg = static_cast&lt;const CB_CallbackMsg*&gt;(msg-&gt;GetData());

    // Invoke the callback on the target thread
    CB_TargetInvoke(callbackMsg);

    // Delete dynamic data passed through message queue
    delete msg;
    break;
}
</pre>

<p>Software locks are handled by the <code>LockGuard </code>module. This file can be updated with locks of your choice, or you can use a different mechanism. Locks are only used in a few places. Define <code>USE_LOCKS</code> within <strong>callback.c</strong> to use <code>LockGuard </code>module locks.&nbsp;</p>

<h2>Which Callback Implementation?</h2>

<p>I&rsquo;ve documented three different asynchronous multicast callback implementations here on CodeProject. Each version has its own unique features and advantages. The sections below highlight the main differences between each solution. See the <strong>References </strong>section below for links to each article.&nbsp;</p>

<h3>Asynchronous Multicast Callbacks in C</h3>

<ul>
	<li>Implemented in C</li>
	<li>Callback function free or static member only</li>
	<li>One callback argument supported</li>
	<li>Callback argument must be a pointer type</li>
	<li>Callback argument data copied with <code>memcpy</code></li>
	<li>Type-safety provided by macros</li>
	<li>Static array holds registered subscriber callbacks</li>
	<li>Number of registered subscribers fixed at compile time</li>
	<li>Fixed block memory allocator in C</li>
	<li>Compact implementation</li>
</ul>

<h3>Asynchronous Multicast Callbacks with Inter-Thread Messaging</h3>

<ul>
	<li>Implemented in C++</li>
	<li>Callback function free or static member only</li>
	<li>One callback argument supported</li>
	<li>Callback argument must be a pointer type</li>
	<li>Callback argument data copied with copy constructor</li>
	<li>Type-safety provided by templates</li>
	<li>Minimal use of templates</li>
	<li>Dynamic list of registered subscriber&nbsp;callbacks</li>
	<li>Number of registered subscribers expands at runtime</li>
	<li>Fixed block memory allocator in C++</li>
	<li>Compact implementation</li>
</ul>

<h3>Asynchronous Multicast Delegates in C++</h3>

<ul>
	<li>Implemented in C++</li>
	<li>C++ delegate paradigm</li>
	<li>Any callback function type (member, static, free)</li>
	<li>Multiple callback arguments supported (up to 5)</li>
	<li>Callback argument any type (value, reference, pointer, pointer to pointer)</li>
	<li>Callback argument&nbsp;data copied with copy constructor</li>
	<li>Type-safety provided by templates</li>
	<li>Heavy use of templates</li>
	<li>Dynamic list of registered subscriber&nbsp;callbacks</li>
	<li>Number of registered subscribers&nbsp;expands at runtime</li>
	<li>Fixed block memory allocator in C++</li>
	<li>Larger implementation&nbsp;</li>
</ul>

<h2>References</h2>

<ul>
	<li><a href="https://www.codeproject.com/Articles/1272619/A-Fixed-Block-Memory-Allocator-in-C">A Fixed Block Memory Allocator in C</a> - by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/1160934/Asynchronous-Multicast-Delegates-in-Cplusplus">Asynchronous Multicast Delegates in C++</a> - by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/1092727/Asynchronous-Multicast-Callbacks-with-Inter-Thread">Asynchronous Multicast Callbacks with Inter-Thread Messaging</a> - by David Lafreniere</li>
</ul>

<h2>Conclusion</h2>

<p>There are many ways to design a publisher/subscriber callback system. This C language version incorporates unique features and eases generating&nbsp;asynchronous callbacks onto a client specified thread of control. The implementation was kept to a minimum to facilitate porting to any system embedded or otherwise.&nbsp;</p>

<p>This callback implementation works on C and C++ projects. However, if you have a strictly C++ project, you could consider using one of the C++ callback implementations listed within the <strong>References </strong>section.&nbsp;</p>

<p>I&#39;ve used this technique on projects with great success. Each module or subsystem may expose one or more outgoing interfaces with <code>CB_DECLARE </code>and <code>CB_DEFINE </code>macros. Any code within the system is able to connect and receive asynchronous callbacks with worrying about cross-threading or the machinery to make it all work. A feature like this eases application design and architecturally standardizes inter-thread communication with a well-understood callback paradigm.</p>