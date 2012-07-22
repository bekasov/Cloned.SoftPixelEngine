/*
 * Thread manager header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_THREADMANAGER_H__
#define __SP_THREADMANAGER_H__


#include "Base/spStandard.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#   include <process.h>
#elif defined(SP_PLATFORM_LINUX)
#   include <pthread.h>
#endif


namespace sp
{


#if defined(SP_PLATFORM_WINDOWS)

typedef DWORD (WINAPI *PFNTHREADPROC)(void* Arguments);
#define THREAD_PROC(n) DWORD WINAPI n(void* Arguments)

#else

typedef void* (*PFNTHREADPROC)(void* Arguments);
#define THREAD_PROC(n) void* n(void* Arguments)

#endif


//! Class for managing multi-threaded procedures.
class SP_EXPORT ThreadManager
{
    
    public:
        
        ThreadManager(PFNTHREADPROC ThreadProc, void* Arguments = 0, bool StartImmediately = true);
        ~ThreadManager();
        
        /* Functions */
        
        //! Returns true if the thread is currently running and returns false if the thread was already terminated.
        bool running() const;
        
        //! Pauses (or rather suspends) the thread.
        void pause();
        
        //! Resumes the paused thread.
        void resume();
        
        //! Terminates the thread execution.
        void terminate();
        
    private:
        
        /* Members */
        
        #if defined(SP_PLATFORM_WINDOWS)
        HANDLE ThreadHandle_;
        #elif defined(SP_PLATFORM_LINUX)
        pthread_t ThreadHandle_;
        #endif
        
};


} // /namepsace sp


#endif



// ================================================================================
