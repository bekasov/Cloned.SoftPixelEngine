/*
 * Timer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spTimer.hpp"
#include "Base/spMemoryManagement.hpp"

#include <ctime>

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#elif defined(SP_PLATFORM_LINUX)
#   include <unistd.h>
#endif


namespace sp
{
namespace io
{


struct SFrequenceQuery
{
    #if defined(SP_PLATFORM_WINDOWS)
    LARGE_INTEGER ClockFrequency;
    LONGLONG StartTick;
    LONGLONG PrevElapsedTime;
    LARGE_INTEGER StartTime;
    #endif
};


Timer::Timer(u64 Duration) :
    StartTime_  (0),
    EndTime_    (0),
    TimeOut_    (0),
    Duration_   (0),
    FreqQuery_  (0)
{
    if (Duration > 0)
        start(Duration);
}
Timer::Timer(bool UseFrequenceQuery) :
    StartTime_  (0),
    EndTime_    (0),
    TimeOut_    (0),
    Duration_   (0),
    FreqQuery_  (0)
{
    if (UseFrequenceQuery)
        createFrequenceQuery();
}
Timer::~Timer()
{
    MemoryManager::deleteMemory(FreqQuery_);
}

void Timer::start(u64 Duration)
{
    StartTime_  = Timer::millisecs();
    EndTime_    = StartTime_ + Duration;
    Duration_   = Duration;
}
void Timer::stop()
{
    StartTime_  = 0;
    EndTime_    = 0;
    Duration_   = 0;
}
void Timer::pause(bool isPaused)
{
    TimeOut_ = (isPaused ? Timer::millisecs() : 0);
}
void Timer::reset()
{
    if (Duration_ > 0)
    {
        StartTime_  = Timer::millisecs();
        EndTime_    = StartTime_ + Duration_;
    }
}

bool Timer::finish()
{
    const u64 Time = Timer::millisecs();
    
    if (paused())
        EndTime_ += Time - TimeOut_;
    
    return EndTime_ > 0 && Time >= EndTime_;
}

// The code for this function was derived from the "Bullet Physics Engine" (http://www.bulletphysics.org/)
u64 Timer::getElapsedMicroseconds()
{
    #if defined(SP_PLATFORM_WINDOWS)
    
    if (!FreqQuery_)
        return 1;
    
    LARGE_INTEGER CurrentTime;
    QueryPerformanceCounter(&CurrentTime);
    LONGLONG ElapsedTime = CurrentTime.QuadPart - FreqQuery_->StartTime.QuadPart;
    
    // Compute the number of millisecond ticks elapsed
    u64 MSecTicks = static_cast<u64>(1000 * ElapsedTime / FreqQuery_->ClockFrequency.QuadPart);
    
    // Check for unexpected leaps in the Win32 performance counter
    // (This is caused by unexpected data across the PCI to ISA
    // bridge, aka south bridge. See Microsoft KB274323.)
    u64 ElapsedTicks = static_cast<u64>(GetTickCount64() - FreqQuery_->StartTick);
    s64 MSecOff = static_cast<s64>(MSecTicks - ElapsedTicks);
    
    if (MSecOff < -100 || MSecOff > 100)
    {
        // Adjust the starting time forwards
        LONGLONG MSecAdjustment = math::Min<LONGLONG>(
            MSecOff * FreqQuery_->ClockFrequency.QuadPart / 1000,
            ElapsedTime - FreqQuery_->PrevElapsedTime
        );
        FreqQuery_->StartTime.QuadPart += MSecAdjustment;
        ElapsedTime -= MSecAdjustment;
    }
    
    // Store the current elapsed time for adjustments next time
    FreqQuery_->PrevElapsedTime = ElapsedTime;
    
    // Convert to microseconds
    return static_cast<u64>(1000000 * ElapsedTime / FreqQuery_->ClockFrequency.QuadPart);
    
    #else
    
    return 1;
    
    #endif
}

void Timer::resetClockCounter()
{
    #if defined(SP_PLATFORM_WINDOWS)
    if (FreqQuery_)
    {
        QueryPerformanceCounter(&FreqQuery_->StartTime);
        FreqQuery_->StartTick = GetTickCount64();
        FreqQuery_->PrevElapsedTime = 0;
    }
    #endif
}

f64 Timer::getFPS()
{
    if (FreqQuery_)
    {
        u64 Time = getElapsedMicroseconds();
        resetClockCounter();
        return 1000000.0 / Time;
    }
    return 60.0;
}

u64 Timer::microsecs()
{
    #if defined(SP_PLATFORM_LINUX)
    return static_cast<u64>(clock());
    #else
    return static_cast<u64>(clock()) * 1000;
    #endif
}

u64 Timer::millisecs()
{
    #if defined(SP_PLATFORM_LINUX)
    return static_cast<u64>(clock()) / 1000;
    #else
    return static_cast<u64>(clock());
    #endif
}

u64 Timer::secs()
{
    return millisecs() / 1000;
}

u32 Timer::getTime(const ETimeTypes Type)
{
    time_t RawTime = time(0);
    tm* Time = localtime(&RawTime);
    
    s32 TimeVal = 0;
    
    switch (Type)
    {
        case TIME_SECOND:
            TimeVal = Time->tm_sec; break;
        case TIME_MINUTE:
            TimeVal = Time->tm_min; break;
        case TIME_HOUR:
            TimeVal = Time->tm_hour; break;
        case TIME_DAY:
            TimeVal = Time->tm_mday; break;
        case TIME_MONTH:
            TimeVal = Time->tm_mon + 1; break;
        case TIME_YEAR:
            TimeVal = Time->tm_year + 1900; break;
    }
    
    return static_cast<u32>(TimeVal);
}

io::stringc Timer::getTime()
{
    return
        io::stringc::number(getTime(TIME_DAY   ), 2) + "/" +
        io::stringc::number(getTime(TIME_MONTH ), 2) + "/" +
        io::stringc        (getTime(TIME_YEAR  )   ) + " " +
        io::stringc::number(getTime(TIME_HOUR  ), 2) + ":" +
        io::stringc::number(getTime(TIME_MINUTE), 2) + ":" +
        io::stringc::number(getTime(TIME_SECOND), 2);
}

f32 Timer::getFPS(u32 UpdateFrameRate)
{
    static u64 LastTime = millisecs();
    static u32 Frames;
    static f32 FPS = 62.0f;
    
    if (++Frames > UpdateFrameRate)
    {
        const u64 CurTime   = millisecs();
        FPS                 = (1000.0f * UpdateFrameRate) / (CurTime - LastTime);
        LastTime            = CurTime;
        Frames              = 0;
    }
    
    return FPS;
}

u32 Timer::getElapsedFrames(u64 Duration)
{
    static u64 LastTime = millisecs();
    static u32 Frames;
    static u32 FPS = 62;
    
    ++Frames;
    
    u64 CurTime = millisecs();
    
    if (CurTime > LastTime + Duration)
    {
        LastTime    = CurTime;
        FPS         = Frames;
        Frames      = 0;
    }
    
    return FPS;
}

void Timer::sleep(u32 Milliseconds)
{
    #if defined(SP_PLATFORM_WINDOWS)
    Sleep(Milliseconds);
    #elif defined(SP_PLATFORM_LINUX)
    usleep(Milliseconds * 1000);
    #endif
}


/*
 * ======= Private: =======
 */

void Timer::createFrequenceQuery()
{
    #if defined(SP_PLATFORM_WINDOWS)
    /* Create frequence query and initialize */
    FreqQuery_ = MemoryManager::createMemory<SFrequenceQuery>("io::Timer::SFrequenceQuery");
    QueryPerformanceFrequency(&FreqQuery_->ClockFrequency);
    resetClockCounter();
    #endif
}


} // /namespace io

} // /namespace sp



// ================================================================================
