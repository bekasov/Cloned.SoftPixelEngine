/*
 * Critical section header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_CRITICALSECTION_H__
#define __SP_CRITICALSECTION_H__


#include "Base/spStandard.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#elif defined(SP_PLATFORM_LINUX) || defined(SP_PLATFORM_IOS)
#   include <pthread.h>
#endif


namespace sp
{


#define CRITICAL_SECTION(s, p)  \
    s.lock();                   \
    p                           \
    s.unlock();


/**
Critical section class used for multi-threading. This class is particular used in the "SecureList" class to
have a thread safe variant of the std::list container.
*/
class SP_EXPORT CriticalSection
{
    
    public:
        
        CriticalSection();
        ~CriticalSection();
        
        /* Functions */
        
        /**
        Locks the critical section. Before this function waits until its unlocked (not used by another thread).
        Don't forget to unlock the section.
        */
        void lock();
        
        //! Unlocks the section.
        void unlock();
        
    private:
        
        /* Members */
        
        #if defined(SP_PLATFORM_WINDOWS)
        mutable CRITICAL_SECTION Section_;
        #elif defined(SP_PLATFORM_LINUX) || defined(SP_PLATFORM_IOS)
        mutable pthread_mutex_t Section_;
        #endif
        
};


} // /namepsace sp


#endif



// ================================================================================
