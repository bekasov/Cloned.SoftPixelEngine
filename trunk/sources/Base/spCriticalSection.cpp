/*
 * Critical section file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spCriticalSection.hpp"


namespace sp
{


#if defined(SP_PLATFORM_WINDOWS)

CriticalSection::CriticalSection()
{
    InitializeCriticalSection(&Section_);
}
CriticalSection::~CriticalSection()
{
    DeleteCriticalSection(&Section_);
}

void CriticalSection::lock()
{
    EnterCriticalSection(&Section_);
}
void CriticalSection::unlock()
{
    LeaveCriticalSection(&Section_);
}

#elif defined(SP_PLATFORM_LINUX)

CriticalSection::CriticalSection()
{
    pthread_mutexattr_t Attribute;
    
    pthread_mutexattr_init(&Attribute);
    pthread_mutexattr_settype(&Attribute, PTHREAD_MUTEX_RECURSIVE);
    
    pthread_mutex_init(&Section_, &Attribute);
}
CriticalSection::~CriticalSection()
{
    pthread_mutex_destroy(&Section_);
}

void CriticalSection::lock()
{
    pthread_mutex_lock(&Section_);
}
void CriticalSection::unlock()
{
    pthread_mutex_unlock(&Section_);
}

#endif


} // /namespace sp



// ================================================================================
