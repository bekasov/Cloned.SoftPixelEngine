/*
 * Thread manager file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spThreadManager.hpp"
#include "Base/spInputOutputLog.hpp"


namespace sp
{


#if defined(SP_PLATFORM_WINDOWS)

ThreadManager::ThreadManager(PFNTHREADPROC ThreadProc, void* Arguments, bool StartImmediately) :
    ThreadHandle_(0)
{
    ThreadHandle_ = CreateThread(
        0, 0, ThreadProc, Arguments, StartImmediately ? 0 : CREATE_SUSPENDED, 0
    );
    
    if (!ThreadHandle_)
        io::Log::error("Could not start thread procedure");
}
ThreadManager::~ThreadManager()
{
    terminate();
}

bool ThreadManager::running() const
{
    if (ThreadHandle_)
    {
        DWORD ExitCode = 0;
        GetExitCodeThread(ThreadHandle_, &ExitCode);
        return ExitCode == STILL_ACTIVE;
    }
    return false;
}

void ThreadManager::pause()
{
    if (ThreadHandle_)
        SuspendThread(ThreadHandle_);
}
void ThreadManager::resume()
{
    if (ThreadHandle_)
        ResumeThread(ThreadHandle_);
}

void ThreadManager::terminate()
{
    if (ThreadHandle_)
    {
        if (TerminateThread(ThreadHandle_, 0))
            ThreadHandle_ = 0;
        else
            io::Log::error("Could not terminate thread");
    }
}

void ThreadManager::setPriority(const EThreadPriorityClasses PriorityClass)
{
    if (ThreadHandle_)
    {
        s32 PriorityFlag = 0;
        
        switch (PriorityClass)
        {
            case THREADPRIORITY_LOW:
                PriorityFlag = THREAD_PRIORITY_LOWEST; break;
            case THREADPRIORITY_NORMAL:
                PriorityFlag = THREAD_PRIORITY_NORMAL; break;
            case THREADPRIORITY_HIGH:
                PriorityFlag = THREAD_PRIORITY_HIGHEST; break;
            default:
                return;
        }
        
        if (!SetThreadPriority(ThreadHandle_, PriorityFlag))
            io::Log::error("Could not set thread priority");
    }
}

#elif defined(SP_PLATFORM_LINUX)

ThreadManager::ThreadManager(PFNTHREADPROC ThreadProc, void* Arguments, bool StartImmediately)
{
    pthread_attr_t Attributes;
    pthread_attr_init(&Attributes);
    pthread_attr_setdetachstate(&Attributes, PTHREAD_CREATE_DETACHED);
    
    if (pthread_create(&ThreadHandle_, &Attributes, ThreadProc, Arguments))
        io::Log::error("Could not start thread procedure");
}
ThreadManager::~ThreadManager()
{
    terminate();
}

bool ThreadManager::running() const
{
    return true; //todo
}

void ThreadManager::pause()
{
    //todo
}
void ThreadManager::resume()
{
    //todo
}

void ThreadManager::terminate()
{
    if (pthread_cancel(ThreadHandle_))
        io::Log::error("Could not terminate thread procedure");
}

void ThreadManager::setPriority(const EThreadPriorityClasses PriorityClass)
{
    //todo
}

#endif


} // /namespace sp



// ================================================================================
