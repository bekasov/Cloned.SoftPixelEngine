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
    DWORD ExitCode = 0;
    GetExitCodeThread(ThreadHandle_, &ExitCode);
    return ExitCode == STILL_ACTIVE;
}

void ThreadManager::pause()
{
    SuspendThread(ThreadHandle_);
}
void ThreadManager::resume()
{
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
    return true; // !TODO!
}

void ThreadManager::pause()
{
    // !TODO!
}
void ThreadManager::resume()
{
    // !TODO!
}

void ThreadManager::terminate()
{
    if (pthread_cancel(ThreadHandle_))
        io::Log::error("Could not terminate thread procedure");
}

#endif


} // /namespace sp



// ================================================================================
